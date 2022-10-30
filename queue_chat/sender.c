#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "header.h"

#define EXIT_CHAR 'e'
#define WRITE_CHAR 't'

struct_service struct_to_send;
struct_chat chat_struct;
struct timespec timeout;

mqd_t wait_connect()
{
    printf("Я зашел в wait_connect\n");
    mqd_t queue_to_connect = -1;
    while (queue_to_connect <= 0)
    {
        printf("я в цикле, жду подключения к серверу\n");
        queue_to_connect = mq_open(SERVICE_QUEUE, O_RDWR);
        if (queue_to_connect == -1)
        {
            perror("mq_open not success\n");
            printf("next try 3..2..1..\n");
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, ФД: %d\n", (int)queue_to_connect);
    return queue_to_connect;
}

int connect_info(mqd_t queue_to_connect, char *argv[])
{
    printf("Я зашел в connect_info\n");
    memset(&struct_to_send, 0 , sizeof(struct_service));
    struct_to_send.pid = getpid();
    memmove(&struct_to_send.nickname, argv[0], sizeof(struct_to_send.nickname));
    if (mq_send(queue_to_connect, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send connect_info not success\n");
        mq_close(queue_to_connect);
        return -1;
    }
    printf("id info has been sent\n");
    return 1;
}

int recive_new_fd(mqd_t queue_to_connect)
{
    printf("Я зашел в recive_new_fd\n");
    int return_recieve = -1;
    char name_chat_queue[SIZE_OF_CHAR_FD];
    char *name_chat_queue_ptr = &name_chat_queue[0];
    while(return_recieve == -1)
    {
        printf("жду ФД для открытия чата\n");
        return_recieve = mq_receive(queue_to_connect, (char *)&chat_struct, sizeof(chat_struct), NULL);
        if (return_recieve == -1)
        {
            perror("recive_new_fd not success\n");
            sleep(10);
        }
    }
    strncpy(name_chat_queue_ptr, chat_struct.new_queue_name, sizeof(name_chat_queue[SIZE_OF_CHAR_FD]));
    printf("new FD = %s\n", name_chat_queue_ptr);
    strcpy(chat_struct.service_message, "New FD was received");
    if (mq_send(queue_to_connect, (char *)&chat_struct, sizeof(chat_struct), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send recive_new_fd not success\n");
        mq_close(queue_to_connect);
        return -1;
    }
    return 1;
}

mqd_t connect_to_chat(char name_chat_queue)
{
    printf("Я зашел в connect_to_chat\n");
    mqd_t chat_queue = mq_open(&name_chat_queue, O_RDWR);
    if (chat_queue == -1)
    {
        perror("chat_queue not open\n");
    }
    return chat_queue;
}

void chat(mqd_t chat_queue)
{
    printf("Я зашел в chat\n");
    printf("if you want exit press %c\n", EXIT_CHAR);
    printf("if you want to write press %c\n", WRITE_CHAR);
    while(getchar() != EXIT_CHAR)
    {
        while(getchar() != WRITE_CHAR)
        {
            if (mq_receive(chat_queue, (char *)&chat_struct, sizeof(chat_struct), NULL) == -1)
            {
                perror("not message to receive\n");
                sleep(1);
                getchar();
            }
            if (chat_struct.pid != struct_to_send.pid)
            {
                printf("%s\n", chat_struct.message);
            }

        }
        printf("Your message:\n");
        fgets(chat_struct.message, MESSAGE_SIZE - sizeof(NICKNAME_SIZE) - 1, stdin);
        char user_pid[NICKNAME_SIZE];
        sprintf(user_pid, "%d", struct_to_send.pid);
        strcat(user_pid, struct_to_send.nickname);
        strcat(chat_struct.message, user_pid);
        printf("sizeof message = %ld\n", sizeof(chat_struct.message));
        if (mq_send(chat_queue, (char *)&chat_struct, sizeof(chat_struct), PRIORITY_OF_QUEUE) == -1)
        {
            perror("mq_send not success\n");
            exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char *argv[])
{
    if ((argc > 2) || (argc < 2))
    {
        printf("write just your nickname\ntry again\n");
        return -1;
    }
    mqd_t queue_to_connect;
    queue_to_connect = wait_connect();
    printf("мой PID = %d, дескриптор очереди = %d\n", getpid(), queue_to_connect);
    if (connect_info(queue_to_connect, &argv[1]) == -1)
    {
        return -1;
    }
    else
    {
        printf("Я успешно передал connect_info\n");
    }
    int name_chat_queue;
    name_chat_queue = recive_new_fd(queue_to_connect);
    if (name_chat_queue != -1)
    {
        printf("я успешно получил name_chat_queue\n");
    }
    mqd_t chat_queue;
    chat_queue = connect_to_chat(name_chat_queue);
    if (chat_queue != -1)
    {
        printf("Я успешно выполнил connect_to_chat\n");
    }
    chat(chat_queue);
    return 0;
}
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "define_and_struct.h"
#include <errno.h>
#include <signal.h>

#define EXIT_CHAR 'e'
#define NUMBER_OF_ARGUMENTS 2
//test

server_to_client_msg_t struct_to_receive = {0};
client_to_server_msg_t struct_to_send = {0};
struct sigevent sigev;

pid_t client_pid;
char nickname[NICKNAME_SIZE];
char client_name_queue_for_chat[CLIENT_QUEUE_SIZE];
char client_pid_queue_for_chat[CLIENT_QUEUE_SIZE];

mqd_t service_queue = -1;
mqd_t chat_queue = -1;

int wait_connect()
{
    printf("Я зашел в wait_connect\n");
    while (service_queue <= 0)
    {
        printf("я в цикле, жду подключения к серверу\n");
        service_queue = mq_open(SERVICE_QUEUE, O_WRONLY);
        if (service_queue == -1)
        {
            perror("mq_open not success\n");
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, queue descriptor: %d\n", (int)service_queue);
    return 0;
}

int connect_info()
{
    printf("Я зашел в connect_info\n");
    memset(&struct_to_send, 0 , sizeof(struct_to_send));
    struct_to_send.action = C2S_ACTION_CONNECT;
    struct_to_send.sender.client_pid = client_pid;
    memmove(&struct_to_send.sender.client_name, &nickname, sizeof(struct_to_send.sender.client_name));
    printf("мой PID = %d\nникнейм = %s\nномер действия = %d\n",\
           struct_to_send.sender.client_pid, struct_to_send.sender.client_name, struct_to_send.action);
    if (mq_send(service_queue, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send connect_info not success\n");
        mq_close(service_queue);
        return -1;
    }
    printf("id info has been sent\n");
    return 0;
}

void create_new_queue_for_chat()
{
    sprintf(client_pid_queue_for_chat, "/User_queue_%d", client_pid);
    printf("user_queue = %s\n", client_pid_queue_for_chat);
    while (chat_queue <= 0)
    {
        printf("я в цикле, жду подключения к чату\n");
        chat_queue = mq_open(client_pid_queue_for_chat, O_RDONLY);
        if (chat_queue == -1)
        {
            printf("error = %d\n", errno);
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, ФД: %d\n", (int)chat_queue);
}

void sig_receive_message()
{
    if (mq_receive(chat_queue, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == sizeof(struct_to_receive))
    {
        mq_notify(chat_queue, &sigev);
    }
    if (struct_to_receive.sender.client_pid != client_pid)
    {
        printf("%s: %s\n", struct_to_receive.sender.client_name, struct_to_receive.server_to_client_msg);
    } 
}

void chat()
{
    printf("Я зашел в chat\n");
    signal(SIGUSR1, sig_receive_message);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    mq_notify(chat_queue, &sigev);
    printf("if you want exit press %c\n", EXIT_CHAR);
    printf("Your message:\n");
    while(getchar() != EXIT_CHAR)
    {
        fgets(struct_to_send.client_to_server_msg, MESSAGE_SIZE, stdin);
        struct_to_send.action = C2S_ACTION_MESSAGE;
        struct_to_send.sender.client_pid = client_pid;
        memmove(&struct_to_send.sender.client_name, &nickname, sizeof(struct_to_send.sender.client_name));
        if (mq_send(service_queue, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
        {
            printf("сообщение не отправилось. Errno = %d\n", errno);
            mq_unlink(client_pid_queue_for_chat);
            exit(EXIT_FAILURE);
        }
    }
    struct_to_send.action = C2S_ACTION_DISCONNECT;
    if (mq_send(service_queue, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send not success\n");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[])
{
    if ((argc > NUMBER_OF_ARGUMENTS) || (argc < NUMBER_OF_ARGUMENTS))
    {
        printf("write just your nickname\ntry again\n");
        return -1;
    }
    memmove(&nickname, argv[1], sizeof(nickname));
    client_pid = getpid();
    wait_connect();
    connect_info();
    create_new_queue_for_chat();
    chat();
    return 0;
}
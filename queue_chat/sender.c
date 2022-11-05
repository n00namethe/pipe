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

server_to_client_msg_t struct_to_receive;
client_to_server_msg_t struct_to_send;
struct sigevent sigev;

pid_t client_pid;
char nickname[NICKNAME_SIZE];
char client_queue_for_chat[CLIENT_QUEUE_SIZE];
char pid_queue[CLIENT_QUEUE_SIZE] = {};
char *pid_queue_ptr = &pid_queue[0];

mqd_t queue_to_connect;
mqd_t chat;

mqd_t wait_connect()
{
    printf("Я зашел в wait_connect\n");
    queue_to_connect = -1;
    while (queue_to_connect <= 0)
    {
        printf("я в цикле, жду подключения к серверу\n");
        queue_to_connect = mq_open(SERVICE_QUEUE, O_WRONLY);
        if (queue_to_connect == -1)
        {
            perror("mq_open not success\n");
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, ФД: %d\n", (int)queue_to_connect);
    return queue_to_connect;
}

int connect_info()
{
    printf("Я зашел в connect_info\n");
    memset(&struct_to_send, 0 , sizeof(struct_to_send));
    struct_to_send.action = 0;
    struct_to_send.sender.client_pid = client_pid;
    memmove(&struct_to_send.sender.client_name, &nickname, sizeof(struct_to_send.sender.client_name));
    printf("мой PID = %d\nникнейм = %s\nномер действия = %d\n",\
            struct_to_send.sender.client_pid, struct_to_send.sender.client_name, struct_to_send.action);
    if (mq_send(queue_to_connect, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send connect_info not success\n");
        mq_close(queue_to_connect);
        return -1;
    }
    printf("id info has been sent\n");
    return 1;
}

void create_new_fd()
{
    sprintf(pid_queue_ptr, "/User_queue_%d", client_pid);
    printf("user_queue = %s\n", pid_queue_ptr);
    chat = -1;
    while (chat <= 0)
    {
        printf("я в цикле, жду подключения к чату\n");
        chat = mq_open(pid_queue_ptr, O_RDONLY);
        if (chat == -1)
        {
            printf("error = %d\n", errno);
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, ФД: %d\n", (int)chat);
}

void sig_receive_message()
{
    mq_notify(chat, &sigev);
    mq_receive(chat, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL);
    if (struct_to_receive.sender.client_pid != client_pid)
    {
        printf("%s: %s\n", struct_to_receive.sender.client_name, struct_to_receive.server_to_client_msg);
    }
    
}

void _chat_()
{
    printf("Я зашел в chat\n");
    signal(SIGUSR1, sig_receive_message);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    mq_notify(chat, &sigev);
    printf("if you want exit press %c\n", EXIT_CHAR);
    printf("Your message:\n");
    while(getchar() != EXIT_CHAR)
    {
        printf("Your message: ");
        fgets(struct_to_send.client_to_server_msg, MESSAGE_SIZE, stdin);
        struct_to_send.action = 2;
        struct_to_send.sender.client_pid = client_pid;
        memmove(&struct_to_send.sender.client_name, &nickname, sizeof(struct_to_send.sender.client_name));
        if (mq_send(queue_to_connect, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
        {
            printf("сообщение не отправилось. Errno = %d\n", errno);
            mq_unlink(pid_queue_ptr);
            exit(EXIT_FAILURE);
        }
    }
    struct_to_send.action = 1;
    if (mq_send(queue_to_connect, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
        {
            perror("mq_send not success\n");
            exit(EXIT_FAILURE);
        }
}


int main(int argc, char *argv[])
{
    if ((argc > 2) || (argc < 2))
    {
        printf("write just your nickname\ntry again\n");
        return -1;
    }
    memmove(&nickname, argv[1], sizeof(nickname));
    client_pid = getpid();
    queue_to_connect = wait_connect();
    connect_info();
    create_new_fd();
    _chat_();
    return 0;
}
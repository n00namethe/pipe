#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "header.h"

#define PRIORITY_OF_QUEUE 1
#define EXIT_CHAR 'e'

int wait_connect(int queue_to_send)
{
    while (queue_to_send <= 0)
    {
        printf("я в цикле, жду подключения к серверу\n");
        queue_to_send = mq_open(MQ_NAME, O_WRONLY, S_IRUSR | S_IWUSR, &attributes);
        if (queue_to_send == -1)
        {
            perror("mq_open not success\n");
            printf("next try 3..2..1..\n");
            sleep(3);
        }
    }
    return queue_to_send;
}

int send_message(int queue_to_send)
{
    message struct_to_send;
    memset(&struct_to_send, 0 , sizeof(message));
    printf("if u want exit - print %c\n", EXIT_CHAR);
    printf("write your nickname:\n");
    scanf("%s", struct_to_send.nickname);
    while(getchar() != EXIT_CHAR)
    {
        printf("Your message:\n");
        fgets(struct_to_send.msg, MESSAGE_SIZE, stdin);
        struct_to_send.pid = getpid();
        if (mq_send(queue_to_send, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
        {
            perror("mq_send not success\n");
            mq_close(queue_to_send);
            return -1;
        }
        printf("message has been sent\n");
    }
}

int main()
{
    mqd_t queue_to_send = 0;
    mqd_t queue_to_recive = 0;
    queue_to_send = wait_connect(queue_to_send);
    
    printf("мой PID = %d, дескриптор очереди = %d\n", getpid(), queue_to_send);
    if (send_message(queue_to_send) == -1)
    {
        return -1;
    }
    return 0;
}
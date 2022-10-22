#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


#define MESSAGE_SIZE 1024
#define NICK_SIZE 32
#define MQ_NAME "/Queue"
#define PRIORITY_OF_QUEUE 1
#define MQ_MAX_NUM_OF_MESSAGES 10

typedef struct
{
    char msg[MESSAGE_SIZE];
    pid_t pid;
    char nickname[NICK_SIZE];
} message;

int main()
{
    struct mq_attr attributes = 
    {
        .mq_flags = 0,                       
        .mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES, 
        .mq_msgsize = sizeof(message),       
        .mq_curmsgs = 0                      
    };
    mqd_t queue = mq_open(MQ_NAME, O_WRONLY, S_IRUSR | S_IWUSR, &attributes);
    if (queue == -1)
    {
        perror("mq_open not success\n");
        return -1;
    }
    message struct_to_send;
    printf("if u want exit - print 'e'\n");
    printf("write your nickname:\n");
    scanf("%s", struct_to_send.nickname);
    while(getchar() != 'e')
    {
        printf("Your message:\n");
        fgets(struct_to_send.msg, MESSAGE_SIZE, stdin);
        struct_to_send.pid = getpid();
        if (mq_send(queue, (char *)&struct_to_send, sizeof(struct_to_send), PRIORITY_OF_QUEUE) == -1)
        {
            perror("mq_send not success\n");
            mq_close(queue);
            return -1;
        }
        else
        {
            printf("message has been sent\n");
        }
    }
    if (mq_close(queue) == -1)
    {
        perror("mq_close not success\n");
        return -1;
    }
    return 0;
}

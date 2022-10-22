#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define MESSAGE_SIZE 1024
#define NICK_SIZE 32
#define MQ_MAX_NUM_OF_MESSAGES 10
#define MQ_NAME "/Queue"
#define WAIT_MESSAGE_TIME 20

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
    struct timespec timeout;
    mqd_t queue = mq_open(MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
    if (queue == -1)
    {
        perror("mq_open not success\n");
        return -1;
    }
    message struct_to_recive;
    printf("wait message just %d seconds\n", WAIT_MESSAGE_TIME);
    while(1)
    {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += WAIT_MESSAGE_TIME;
        timeout.tv_nsec = 0;
        if (mq_timedreceive(queue, (char *)&struct_to_recive, sizeof(struct_to_recive), NULL, &timeout) == -1)
        {
            if (mq_close(queue) == -1)
            {
                perror("mq_close not success\n");
                return -1;
            }
            if (mq_unlink(MQ_NAME) == -1)
            {
                perror("mq_unlink not success\n");
                return -1;
            }
        }
        printf("%d %s: %s\n", struct_to_recive.pid, struct_to_recive.nickname, struct_to_recive.msg);
    }
    return 0;
}

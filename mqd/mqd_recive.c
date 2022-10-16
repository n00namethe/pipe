#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <string.h>
#include <stdio.h>

#define MAX 1024
#define MQ_NAME "/Queue"

typedef struct
{
    char msg[MAX];
} message;

int main()
{
    struct mq_attr attributes = 
        {
            .mq_flags = 0,                       /* Flags (ignored for mq_open()) */
            .mq_maxmsg = 10,                     /* Max. # of messages on queue */
            .mq_msgsize = sizeof(message),       /* Max. message size (bytes) */
            .mq_curmsgs = 0                      /* # of messages currently in queue (ignored for mq_open()) */
        };
    mqd_t queue = mq_open(MQ_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attributes);
    if (queue == -1)
    {
        perror("mq_open not success\n");
        return(-1);
    }
    message struct_to_recive;
    if (mq_receive(queue, (char *)&struct_to_recive, sizeof(struct_to_recive), NULL) == -1)
    {
        perror("mq_receive not success\n");
        return(-1);
    }

    printf("%s\n", struct_to_recive.msg);

    printf("press any key to finish\n");
    getchar();

    if (mq_close(queue) == -1)
    {
        perror("mq_close not success\n");
        return(-1);
    }
    if (mq_unlink(MQ_NAME) == -1)
    {
        perror("mq_unlink not success\n");
        return(-1);
    }

    return 0;
}

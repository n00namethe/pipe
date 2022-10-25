#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "header.h"

#define WAIT_MESSAGE_TIME 15

int main()
{
    struct timespec timeout;
    mqd_t queue = mq_open(MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
    if (queue == -1)
    {
        perror("mq_open not success\n");
        return -1;
    }
    message struct_to_recive;
    memset(&struct_to_recive, 0, sizeof(message));
    printf("я сервер мой PID: %d\n", getpid());
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
            return 0;
        }
        printf("%d %s: %s\n", struct_to_recive.pid, struct_to_recive.nickname, struct_to_recive.msg);
    }
    return 0;
}

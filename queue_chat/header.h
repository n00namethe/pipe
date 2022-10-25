#pragma once

#define MESSAGE_SIZE 1024
#define NICKNAME_SIZE 32
#define MQ_NAME "/Queue"
#define MQ_MAX_NUM_OF_MESSAGES 10
#define DEFAULT_VALUE 0

typedef struct
{
    char msg[MESSAGE_SIZE];
    pid_t pid;
    char nickname[NICKNAME_SIZE];
} message;

struct mq_attr attributes = 
{
        .mq_flags = DEFAULT_VALUE,
        .mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES,
        .mq_msgsize = sizeof(message),
        .mq_curmsgs = DEFAULT_VALUE
};
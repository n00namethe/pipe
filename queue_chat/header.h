#pragma once

#define MESSAGE_SIZE 1024
#define NICKNAME_SIZE 64
#define SERVICE_QUEUE "/Queue_serv"
#define MQ_MAX_NUM_OF_MESSAGES 10
#define DEFAULT_VALUE 0
#define PRIORITY_OF_QUEUE 1
#define SIZE_OF_CHAR_FD 64

typedef struct 
{
    pid_t pid;
    char nickname[NICKNAME_SIZE];
    
} struct_service;

typedef struct
{
    char message[MESSAGE_SIZE];
    pid_t pid;
    char new_queue_name[SIZE_OF_CHAR_FD];
    char service_message[MESSAGE_SIZE];
} struct_chat;


struct mq_attr attributes = 
{
        .mq_flags = DEFAULT_VALUE,
        .mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES,
        .mq_msgsize = sizeof(struct_chat),
        .mq_curmsgs = DEFAULT_VALUE
};
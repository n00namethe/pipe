#pragma once

#define MESSAGE_SIZE 1024
#define NICKNAME_SIZE 64
#define CLIENT_QUEUE_SIZE 64
#define SERVICE_QUEUE "/Queue_serv"
#define MQ_MAX_NUM_OF_MESSAGES 10
#define DEFAULT_VALUE 0
#define PRIORITY_OF_QUEUE 1

typedef struct _user_id_t
{
    pid_t client_pid;
    char client_name[NICKNAME_SIZE];
} user_id_t;

typedef enum _chat_action_t
{
    C2S_ACTION_INVALID_TYPE = -1,
    C2S_ACTION_CONNECT = 0,
    C2S_ACTION_DISCONNECT,
    C2S_ACTION_MESSAGE,
    C2S_ACTION_END_OF_ENUM
} chat_action_t;

typedef struct _client_to_server_msg_t
{
    chat_action_t action;
    user_id_t sender;
    char client_to_server_msg[MESSAGE_SIZE];
} client_to_server_msg_t;

typedef struct _server_to_client_msg_t
{
    user_id_t sender;
    char server_to_client_msg[MESSAGE_SIZE];
} server_to_client_msg_t;

struct mq_attr attributes_from_client_to_server = 
{
        .mq_flags = DEFAULT_VALUE,
        .mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES,
        .mq_msgsize = sizeof(client_to_server_msg_t),
        .mq_curmsgs = DEFAULT_VALUE
};

struct mq_attr attributes_from_server_client = 
{
        .mq_flags = DEFAULT_VALUE,
        .mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES,
        .mq_msgsize = sizeof(server_to_client_msg_t),
        .mq_curmsgs = DEFAULT_VALUE
};
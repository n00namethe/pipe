#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include "header.h"

#define WAIT_MESSAGE_TIME 5
#define NUMBER_OF_USERS 10
#define CHAT_QUEUE "/chat_queue"

struct timespec timeout;
struct_service new_user_id;
struct_chat chat_struct;
int user_number = 1;
int *user_number_ptr = &user_number;

typedef struct
{
    pid_t pid;
    char nickname[NICKNAME_SIZE];
} users_base;
users_base users[NUMBER_OF_USERS] = {};

mqd_t create_queue_service()
{
    printf("я зашел в create_queue_service\n");
    mqd_t serv_queue = mq_open(SERVICE_QUEUE, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
    if (serv_queue == -1)
    {
        perror("ошибка открытия очереди SERVICE_QUEUE");
        return -1;
    }
    return serv_queue;
}

int new_user_connect(mqd_t serv_queue)
{
    struct timespec timeout;
    printf("я зашел в new_user_connect\n");
    memset(&new_user_id, 0, sizeof(struct_service));
    while(1)
    {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += WAIT_MESSAGE_TIME;
        timeout.tv_nsec = 0;
        printf("у вас есть %d секунд чтобы добавиться в чат\n", WAIT_MESSAGE_TIME);
        if (mq_timedreceive(serv_queue, (char *)&new_user_id, sizeof(new_user_id), NULL, &timeout) == -1)
        {
            perror("время для подключения закончилось, иду в main serv_queue\n");
            break;
        }
        users[*user_number_ptr].pid = new_user_id.pid;
        strncpy(users[*user_number_ptr].nickname, new_user_id.nickname, sizeof(new_user_id.nickname));
        printf("pid from sender: %d\n", new_user_id.pid);
        printf("nickname from sender: %s\n", new_user_id.nickname);
        printf("new users[%d].pid = %d\n", *user_number_ptr, users[*user_number_ptr].pid);
        printf("new users[%d].nickname = %s\n", *user_number_ptr, users[*user_number_ptr].nickname);
        *user_number_ptr += 1;
    }
    return 1;
}

mqd_t create_queue_chat()
{
    printf("я зашел в create_queue_chat\n");
    mqd_t chat = mq_open(CHAT_QUEUE, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
    if (chat == -1)
    {
        perror("ошибка открытия очереди для чата\n");
        return -1;
    }
    return chat;
}

int send_new_FD(mqd_t serv_queue)
{
    printf("Я зашел в send_new_FD\n");
    int return_recieve = -1;
    strncpy(chat_struct.new_queue_name, CHAT_QUEUE, sizeof(chat_struct.new_queue_name));
    printf("name_chat_queue = %s\n", chat_struct.new_queue_name);
    if (mq_send(serv_queue, (char *)&chat_struct, sizeof(chat_struct), PRIORITY_OF_QUEUE) == -1)
    {
        perror("mq_send new FD not success\n");
        mq_close(serv_queue);
        mq_unlink(SERVICE_QUEUE);
        return -1;
    }
    while(return_recieve == -1)
    {
        printf("жду инфо о получении ФД для открытия чата\n");
        return_recieve = mq_receive(serv_queue, (char *)&chat_struct, sizeof(chat_struct), NULL);
        if (return_recieve == -1)
        {
            perror("recive_new_fd not success\n");
            sleep(10);
        }
        printf("%s\n", chat_struct.service_message);
    }
    printf("name_chat_queue = %s\n", chat_struct.service_message);
    return 1;
}

int main()
{
    
    mqd_t serv_queue;
    serv_queue = create_queue_service();
    if (serv_queue != -1)
    {
        printf("очередь serv_queue успешнот открыта, ФД = %d\n", serv_queue);
    }
    if (new_user_connect(serv_queue) != -1)
    {
        printf("Все пользователи к очереди serv_queue присоединились\n");
    }
    mqd_t chat;
    chat = create_queue_chat();
    if (chat == -1)
    {
        perror("create_queue_chat not success, go to exit\n");
        mq_close(serv_queue);
        mq_unlink(SERVICE_QUEUE);
    }
    else
    {
        printf("новая очередь chat = %d успешно открыта\n", chat);
    }
    
    if (send_new_FD(chat) == -1)
    {
        perror("send_new_FD not success\n");
        return -1;
    }
    else
    {
        printf("я успешно send_new_FD\n");
    }
    if (mq_close(serv_queue) == -1)
    {
        perror("mq_close serv_queue not success\n");
        return -1;
    }
    if (mq_unlink(SERVICE_QUEUE) == -1)
    {
        perror("mq_unlink SERVICE_QUEUE not success\n");
        return -1;
    }
    return 0;
}
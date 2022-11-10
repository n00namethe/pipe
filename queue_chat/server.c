#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "define_and_struct.h"

#define WAIT_MESSAGE_TIME 5
#define NUMBER_OF_USERS 10

mqd_t serv_queue;

client_to_server_msg_t service_struct;
server_to_client_msg_t chat_struct;
typedef struct _client_t
{
    pid_t pid;
    char nickname[NICKNAME_SIZE];
    char client_queue[CLIENT_QUEUE_SIZE];
    mqd_t chat_queue;
} client_t;
client_t users_db[NUMBER_OF_USERS] = {0};

int create_queue_service()
{
    printf("я зашел в create_queue_service\n");
    serv_queue = mq_open(SERVICE_QUEUE, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes_from_client_to_server);
    if (serv_queue == -1)
    {
        perror("ошибка открытия очереди SERVICE_QUEUE");
        return -1;
    }
    printf("очередь serv_queue успешнот открыта, ФД = %d\n", (int)serv_queue);
    memset(&service_struct, 0, sizeof(service_struct));
    return 0;
}

int case_connect()
{
    for (int i = 0; i < NUMBER_OF_USERS; i++)
    {
        if (i == NUMBER_OF_USERS - 1)
        {
            printf("Свободных мест нет, количество пользователей = %d\n", NUMBER_OF_USERS);
            break;
        }
        else if (users_db[i].pid == 0)
        {
            users_db[i].pid = service_struct.sender.client_pid;
            strncpy(users_db[i].nickname, service_struct.sender.client_name, sizeof(service_struct.sender.client_name));
            printf("user[%d].pid = %d\n", i, users_db[i].pid);
            printf("user[%d].nickname = %s\n", i, users_db[i].nickname);
            sprintf(users_db[i].client_queue, "/User_queue_%d", users_db[i].pid);

            printf("users_db[%d].client_queue = %s\n", i, users_db[i].client_queue);

            users_db[i].chat_queue = mq_open(users_db[i].client_queue, O_CREAT | O_NONBLOCK | O_RDWR, S_IRUSR | S_IWUSR, &attributes_from_server_client);
            if (users_db[i].chat_queue == -1)
            {
                printf("error = %d\n", errno);
                mq_close(serv_queue);
                mq_unlink(SERVICE_QUEUE);
                return 1;
            }
            printf("chat_queue[%d] создан, FD = %d\n", i, (int)users_db[i].chat_queue);
            break;
        }
    }
    return 0;
}

int case_disconnect()
{
    for (int i = 0; i < NUMBER_OF_USERS; i++)
    {
        if (users_db[i].pid == service_struct.sender.client_pid)
        {
            users_db[i].pid = 0;
            strncpy(users_db[i].nickname, "0", sizeof(users_db[i].nickname));
            if (mq_close(users_db[i].chat_queue) == -1)
            {
                printf("Не получилось закрыть очередь chat_queue[%d]. Errno = %d\n", i, errno);
            }
            if (mq_unlink(users_db[i].client_queue) == -1)
            {
                printf("Не получилось unlink очередь chat_queue[%d]. Errno = %d\n", i, errno);
            }
            break;
        }
    }
    int count_activ_user = 0;
    printf("список пользователей после удаления:\n");
    for (int i = 0; i < NUMBER_OF_USERS; i++)
    {
        if (users_db[i].pid != 0)
        {
            printf("%d: %d %s\n", i, users_db[i].pid, users_db[i].nickname);
            count_activ_user++;
        }
    }
    printf("Activ user count = %d\n", count_activ_user);
    if (count_activ_user == 0)
    {
        return 1;
    }
    return 0;
}

int receive_service_struct()
{
    while(1)
    {
        int size_receive = mq_receive(serv_queue, (char *)&service_struct, sizeof(service_struct), NULL);
        if (size_receive < 0)
        {
            printf("размер сообщения меньше 0 байт. errno = %d\n", errno);
            printf("повтор попытки через 2...1...\n");
            sleep(3);
            continue;
        }
        if (size_receive != sizeof(service_struct))
        {
            printf("размер полученного сообщения меньше ожидаемого. errno = %d\n", errno);
            printf("повтор попытки через 2...1...\n");
            sleep(3);
            continue;
        }
        switch(service_struct.action)
        {
            case C2S_ACTION_CONNECT:
            {
                printf("case C2S_ACTION_CONNECT\n");
                if (case_connect() == 1)
                {
                    return 0;
                }
                break;
            }
            case C2S_ACTION_DISCONNECT:
            {
                printf("case C2S_ACTION_DISCONNECT\n");
                if (case_disconnect() == 1)
                {
                    return 0;
                }
                break;
            }
            case C2S_ACTION_MESSAGE:
            { 
                printf("case C2S_ACTION_MESSAGE\n");
                strncpy(chat_struct.server_to_client_msg, service_struct.client_to_server_msg, sizeof(chat_struct.server_to_client_msg));
                strncpy(chat_struct.sender.client_name, service_struct.sender.client_name, sizeof(chat_struct.sender.client_name));
                chat_struct.sender.client_pid = service_struct.sender.client_pid;
                for(int i = 0; i < NUMBER_OF_USERS - 1; i++)
                {
                    if (users_db[i].pid != 0)
                    {
                        if (mq_send(users_db[i].chat_queue, (char *)&chat_struct, sizeof(chat_struct), PRIORITY_OF_QUEUE) == -1)
                        {
                            printf("отправить сообщение пользователю users_db[%d].client_queue не удалось. Errno = %d\n", i, errno);
                            break;
                        }
                    }
                }
                break;
            }
            default:
            {
                printf("Получен неопознанный тип события. %d\n", service_struct.action);
                break;
            }
        }
    }
    return 0;
}

int main()
{
    create_queue_service();
    receive_service_struct();
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
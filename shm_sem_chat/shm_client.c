#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "struct_and_define.h"
#include <errno.h>
#include <semaphore.h>
#include <signal.h>

#define EXIT_CHAR 'e'
#define NUMBER_OF_ARGUMENTS 2

//client_context_t client_ctx;
struct sigevent sigev;

typedef struct _client_context_t
{
    client_to_server_msg_t *client2server_msg;
    sem_t *sem_service;
    user_id_t client_id;
    char client_pid_queue_for_chat[CLIENT_QUEUE_SIZE];
    mqd_t chat_queue;
} client_context_t;
client_context_t client_ctx = {0};

int shm_serv;

int semaphore_value()
{
    int sval;
    int *sval_ptr = &sval;
    sem_getvalue(client_ctx.sem_service, sval_ptr);
    return sval;
}

void semaphore_post(const char *asker_func_name)
{
    printf("%s делает sem_post\n", asker_func_name);
    printf("значение семафора до %s = %d\n", asker_func_name, semaphore_value());
    sem_post(client_ctx.sem_service);
}

void semaphore_wait(const char *asker_func_name)
{
    printf("%s делает sem_wait\n", asker_func_name);
    printf("значение семафора до %s = %d\n", asker_func_name, semaphore_value());
    sem_wait(client_ctx.sem_service);
}

void check_memory_free_for_sem_wait()
{
    while(1)
    {
        semaphore_wait(__FUNCTION__);
        if (client_ctx.client2server_msg->action != C2S_ACTION_INVALID_TYPE)
        {
            printf("память не высвобождена, иди своей дорогой\n");
            semaphore_post(__FUNCTION__);
            sleep(1);
            continue;
        }
        break;
    }
}

void my_printf(const char *asker_func_name)
{
    printf("я зашел в %s\n", asker_func_name);
}

void global_client_name_set(int argc, char argv[0])
{
    my_printf(__FUNCTION__);
    if ((argc > NUMBER_OF_ARGUMENTS) || (argc < NUMBER_OF_ARGUMENTS))
    {
        printf("write just your nickname\ntry again\n");
        exit(EXIT_FAILURE);
    }
    memmove(client_ctx.client_id.client_name, &argv[0], sizeof(client_ctx.client_id.client_name));
}

void global_client_pid_set()
{
    my_printf(__FUNCTION__);
    client_ctx.client_id.client_pid = getpid();
}

void wait_connect()
{
    my_printf(__FUNCTION__);
    printf("Я зашел в wait_connect\n");
    shm_serv = -1;
    while (shm_serv <= 0)
    {
        printf("я в цикле, жду подключения к серверу\n");
        shm_serv = shm_open(SERVICE_SHM, O_RDWR, S_IRUSR | S_IWUSR);
        if (shm_serv == -1)
        {
            perror("shm_open not success\n");
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    if (ftruncate(shm_serv, sizeof(client_to_server_msg_t)) == -1)
    {
        printf("ошибка ftruncate shm_serv: %d\n", errno);
        close(shm_serv);
        exit(EXIT_FAILURE);
    }
    client_ctx.client2server_msg = mmap(NULL, sizeof(client_to_server_msg_t), PROT_WRITE, MAP_SHARED, shm_serv, 0);
    if (client_ctx.client2server_msg == MAP_FAILED)
    {
        printf("ошибка mmap: %d\n", errno);
        close(shm_serv);
        exit(EXIT_FAILURE);
    }
    client_ctx.sem_service = sem_open(SEM, O_RDWR, S_IRWXU, 0);
    if(client_ctx.sem_service == SEM_FAILED)
    {
        perror("sem1 open not success\n");
        close(shm_serv);
        exit(EXIT_FAILURE);
    }
    printf("Доступ к общей памяти получен успешно, shm descriptor: %d\n", shm_serv);
}

int connect_info()
{
    my_printf(__FUNCTION__);
    check_memory_free_for_sem_wait();
    client_ctx.client2server_msg->action = C2S_ACTION_CONNECT;
    client_ctx.client2server_msg->client_id.client_pid = client_ctx.client_id.client_pid;
    sprintf(client_ctx.client2server_msg->client_id.client_name, "%s", client_ctx.client_id.client_name);
    semaphore_post(__FUNCTION__);
    printf("мой PID = %d\nникнейм = %s\nномер действия = %d\n", \
           client_ctx.client_id.client_pid, client_ctx.client_id.client_name, client_ctx.client2server_msg->action);
    return 1;
}

void create_new_fd()
{
    my_printf(__FUNCTION__);
    sprintf(client_ctx.client_pid_queue_for_chat, "/User_queue_%d", client_ctx.client_id.client_pid);
    printf("user_queue = %s\n", client_ctx.client_pid_queue_for_chat);
    client_ctx.chat_queue = -1;
    while (client_ctx.chat_queue <= 0)
    {
        printf("я в цикле, жду подключения к чату\n");
        client_ctx.chat_queue = mq_open(client_ctx.client_pid_queue_for_chat, O_RDONLY);
        if (client_ctx.chat_queue == -1)
        {
            printf("error = %d\n", errno);
            printf("next try 3..2..1..\nerrno = %d\n", errno);
            sleep(3);
        }
    }
    printf("Подкдючение к очереди произошло успешно, queue descriptor: %d\n", (int)client_ctx.chat_queue);
}

void sig_receive_message()
{
    server_to_client_msg_t struct_to_receive = {0};
    mq_notify(client_ctx.chat_queue, &sigev);
    mq_receive(client_ctx.chat_queue, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL);
    if (struct_to_receive.client_id.client_pid != client_ctx.client_id.client_pid)
    {
        printf("%s: %s\n", struct_to_receive.client_id.client_name, struct_to_receive.server_to_client_msg);
    }
    
}

void disconnect()
{
    my_printf(__FUNCTION__);
    while(1)
    {
        check_memory_free_for_sem_wait();
        client_ctx.client2server_msg->client_id.client_pid = client_ctx.client_id.client_pid;
        client_ctx.client2server_msg->action = C2S_ACTION_DISCONNECT;
        semaphore_post(__FUNCTION__);
        break;
    }
}

void send_message_to_chat()
{
    my_printf(__FUNCTION__);
    printf("if you want exit press %c\n", EXIT_CHAR);
    printf("Your message:\n");
    while(getchar() != EXIT_CHAR)
    {
        check_memory_free_for_sem_wait();
        fgets(client_ctx.client2server_msg->client_to_server_msg, MESSAGE_SIZE, stdin);
        client_ctx.client2server_msg->action = C2S_ACTION_MESSAGE;
        client_ctx.client2server_msg->client_id.client_pid = client_ctx.client_id.client_pid;
        sprintf(client_ctx.client2server_msg->client_id.client_name, "%s", client_ctx.client_id.client_name);
        semaphore_post(__FUNCTION__);
    }
}

void chat()
{
    my_printf(__FUNCTION__);
    printf("Я зашел в chat\n");
    signal(SIGUSR1, sig_receive_message);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    mq_notify(client_ctx.chat_queue, &sigev);
    send_message_to_chat();
    disconnect();
}

int main(int argc, char *argv[])
{
    global_client_name_set(argc, argv[1]);
    global_client_pid_set();
    wait_connect();
    connect_info();
    create_new_fd();
    chat();
    return 0;
}
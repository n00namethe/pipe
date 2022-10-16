#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    char *buffer = "Message to named pipe\n";
    printf("Отправил клиенту сообщение: %s\n", buffer);
    int fd_fifo = open("file.buf", O_WRONLY | O_CREAT);

    if (mkfifo("file.buf", S_IRWXU) == -1)
    {
        perror("Невозможно создать fifo");
        exit(EXIT_FAILURE);
    }
    if (fd_fifo == -1)
    {
        perror("Невозможно открыть fifo");
        exit(EXIT_FAILURE);
    }
    write(fd_fifo, buffer, strlen(buffer));
    close(fd_fifo);
    exit(EXIT_SUCCESS);
}

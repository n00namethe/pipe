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
    char buf[100];
    int fd_fifo = open("file.buf", O_RDONLY);
    if (fd_fifo == -1)
    {
        perror("Невозможно открыть fifo");
        exit(EXIT_FAILURE);
    }

    if (read(fd_fifo, &buf, sizeof(buf)) == -1)
    {
        perror("Невозможно прочесть из fifo");
        exit(EXIT_FAILURE);
    } else 
    {
        printf("Прочитано из fifo: %s\n", buf);
    }

    printf("Чтение закончено\n");
    close(fd_fifo);
    exit(EXIT_SUCCESS);
}

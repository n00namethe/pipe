#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MY_PRINTF(...) printf("[%d] ", getpid()); (printf(__VA_ARGS__))

int main()
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Creating pipe error");
        exit(EXIT_FAILURE);
    }
    MY_PRINTF("sucsess creaing pipe, read %d, write %d\n", pipefd[0], pipefd[1]);
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        MY_PRINTF("im a child, read from pipe %d\n", pipefd[0]);
        close(pipefd[1]);
        char ch;
        while(read(pipefd[0], &ch, 1) > 0) {
            MY_PRINTF("get char from pipe '%c'\n", ch);
        }
        MY_PRINTF("close child\n");
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    } 
    else {
        char* str="Message to pipe!";
        MY_PRINTF("im a parent, write '%s' to pipe %d\n", str, pipefd[1]);
        close(pipefd[0]);
        write(pipefd[1], str, strlen(str));
        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}
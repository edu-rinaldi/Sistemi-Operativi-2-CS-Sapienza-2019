#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int main(void)
{
    int file_descriptor[2];
    int fd[2];
    pipe(file_descriptor);
    pipe(fd);
    pid_t pid = fork();

    // entro nel processo figlio
    if(pid == 0)
    {
        close(file_descriptor[0]);
        close(fd[0]);
        dup2(file_descriptor[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        char *argv[] = {(char *)"gawk", "/^s/{print $0}", "test.txt", NULL};
        // char *argv[] = {(char *)"ls", NULL};
        execvp(argv[0], argv);
        // char *argv2[] = {(char *)"ls", NULL};
        // execvp(argv2[0], argv2);
    }
    else
    {
        close(file_descriptor[1]);
        close(fd[1]);
        dup2(file_descriptor[0], STDIN_FILENO);
        char buff[100] = "";
        char buff2[100] = "";
        read(STDIN_FILENO, buff, 99);
        read(fd[0], buff2, 99);

        printf("STDOUT: %s\n", buff);
        printf("STDERR: %s\n", buff2);

        int status;
        pid_t wpid = waitpid(pid, &status, 0);
        return wpid == pid && WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}
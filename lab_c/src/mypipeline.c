#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pipefd[2];
    pid_t cpid1, cpid2;

    fprintf(stderr, "(parent_process>forking…)\n");
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    // first child
    cpid1 = fork();
    if (cpid1 < 0)
    {
        perror("fork");
        exit(1);
    }

    if (cpid1 == 0)
    {
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[1]);
        close(pipefd[0]);
        char *cmd1[] = {"ls", "-lsa", NULL};
        fprintf(stderr, "(child1>going to execute cmd: ls -lsa)\n");
        execvp(cmd1[0], cmd1);
        perror("execvp");
        exit(1);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid1);
    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipefd[1]);

    // second child
    cpid2 = fork();
    if (cpid2 < 0)
    {
        perror("fork");
        exit(1);
    }

    if (cpid2 == 0)
    {
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        close(STDIN_FILENO);
        dup(pipefd[0]);
        close(pipefd[0]);
        char *cmd2[] = {"tail", "-n", "3", NULL};
        fprintf(stderr, "(child2>going to execute cmd: tail -n 3)\n");
        execvp(cmd2[0], cmd2);
        perror("execvp");
        exit(1);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipefd[0]);
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);
    fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "LineParser.h"

// ref
int execute(cmdLine *pCmdLine)
{
    return execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

int debug_mode = 0;

// Helper function to handle Input/Output redirection
void handle_redirection(cmdLine *cmd)
{
    // 1. Input Redirection (<)
    if (cmd->inputRedirect != NULL)
    {
        int input_fd = open(cmd->inputRedirect, O_RDONLY);
        if (input_fd < 0)
        {
            perror("Input redirection failed");
            freeCmdLines(cmd);
            _exit(1);
        }
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2 input failed");
            _exit(1);
        }
        close(input_fd);
    }

    // 2. Output Redirection (>)
    if (cmd->outputRedirect != NULL)
    {
        int output_fd = open(cmd->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd < 0)
        {
            perror("Output redirection failed");
            freeCmdLines(cmd);
            _exit(1);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 output failed");
            _exit(1);
        }
        close(output_fd);
    }
}

int main(int argc, char **argv)
{
    // argv handling
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
            debug_mode = 1;
    }

    while (1)
    {
        // ref: https://pubs.opengroup.org/onlinepubs/007904975/functions/getcwd.html
        char cwdbuf[PATH_MAX];

        if (getcwd(cwdbuf, PATH_MAX) == NULL)
        {
            perror("getcwd failed");
            return 1;
        }
        printf("%s ", cwdbuf);

        char userinput[2048];
        if (fgets(userinput, 2048, stdin) != NULL)
        {
            if (strcmp(userinput, "quit\n") == 0)
                break;

            cmdLine *cmd = parseCmdLines(userinput);

            // cd handling
            if (strcmp(cmd->arguments[0], "cd") == 0)
            {
                if (chdir(cmd->arguments[1]) != 0)
                    fprintf(stderr, "cd failed\n");

                freeCmdLines(cmd);
                continue;
            }

            int child_pid = fork();

            // if we're the child process
            if (child_pid == 0)
            {
                // debug info printing
                if (debug_mode)
                {
                    fprintf(stderr, "PID: %d\n", getpid());
                    fprintf(stderr, "Executing command: %s\n", cmd->arguments[0]);
                }

                handle_redirection(cmd);

                // command execution
                if (execute(cmd) == -1)
                {
                    freeCmdLines(cmd);
                    perror("execv failed");
                    _exit(1);
                }
            }

            // if we're the parent process
            else
            {
                if (cmd->blocking)
                    waitpid(child_pid, NULL, 0);
            }

            freeCmdLines(cmd);
        }
    }

    return 0;
}
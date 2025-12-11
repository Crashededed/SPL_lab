#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"

int debug_mode = 0;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    struct process *new_process = malloc(sizeof(process));
    if (new_process == NULL)
    {
        perror("malloc failed");
        return;
    }
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;
    while (curr != NULL)
    {
        if (curr->pid == pid)
        {
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
}

void updateProcessList(process **process_list)
{
    process *curr = *process_list;
    while (curr != NULL)
    {
        // If it returns the PID, the process is a ZOMBIE (finished).
        // If it returns 0, the process is still running.
        // If it returns -1, the process is gone (or error).
        pid_t result = waitpid(curr->pid, NULL, WNOHANG);

        if (result == -1 || result == curr->pid)
        {
            updateProcessStatus(*process_list, curr->pid, TERMINATED);
        }
        curr = curr->next;
    }
}

void printProcessList(process **process_list)
{
    printf("%-10s %-15s %-10s\n", "PID", "Command", "STATUS");
    updateProcessList(process_list);

    process *curr = *process_list;
    process *prev = NULL;

    while (curr != NULL)
    {
        printf("%-10d %-15s %-10s\n",
               curr->pid,
               curr->cmd->arguments[0],
               curr->status == RUNNING ? "RUNNING" : curr->status == SUSPENDED ? "SUSPENDED"
                                                                               : "TERMINATED");

        if (curr->status == TERMINATED)
        {
            // Remove from list
            if (prev == NULL)
                *process_list = curr->next;
            else
                prev->next = curr->next;

            process *to_delete = curr;
            curr = curr->next;

            // Free memory
            freeCmdLines(to_delete->cmd);
            free(to_delete);
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}

void freeProcessList(process **process_list)
{
    struct process *current = *process_list;
    while (current != NULL)
    {
        struct process *next = current->next;
        freeCmdLines(current->cmd);
        free(current);
        current = next;
    }
}

int execute(cmdLine *pCmdLine)
{
    return execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

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

// check if a given string is a number
int is_number(const char *s)
{
    for (int i = 0; s[i]; i++)
        if (s[i] < '0' || s[i] > '9')
            return 0;
    return 1;
}

// helper command to recognize and handle zzzz, kuku, blast commands
int handle_process_commands(cmdLine *cmd, process **process_list)
{
    // check for form: command + PID
    if (cmd->argCount != 2 || !is_number(cmd->arguments[1]))
        return 0;
    int pid = atoi(cmd->arguments[1]);
    if (strcmp(cmd->arguments[0], "zzzz") == 0)
    {
        if (kill(pid, SIGSTOP) == -1)
            perror("zzzz failed");
        else
        {
            printf("Process %d suspended (SIGSTOP)\n", pid);
            updateProcessStatus(*process_list, pid, SUSPENDED);
        }
        return 1;
    }

    if (strcmp(cmd->arguments[0], "kuku") == 0)
    {
        if (kill(pid, SIGCONT) == -1)
            perror("kuku failed");
        else
        {
            printf("Process %d continued (SIGCONT)\n", pid);
            updateProcessStatus(*process_list, pid, RUNNING);
        }
        return 1;
    }

    if (strcmp(cmd->arguments[0], "blast") == 0)
    {
        if (kill(pid, SIGINT) == -1)
            perror("blast failed");
        else
            printf("Process %d blasted (SIGINT)\n", pid);
            // no need to update status to TERMINATED here,
            // it will be updated in the next printProcessList call
        return 1;
    }
    return 0;
}

void execute_pipeline(cmdLine *left, process **process_list)
{
    cmdLine *right = left->next;
    left->next = NULL; // detach right side to avoid double free

    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }
    pid_t cpid1, cpid2;

    cpid1 = fork();
    if (cpid1 < 0)
    {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    if (cpid1 == 0)
    {
        // CHILD 1
        if (!left->blocking)
        {
            setpgid(0, 0);
        }

        if (debug_mode)
        {
            fprintf(stderr, "PID: %d (child1)\n", getpid());
            fprintf(stderr, "Executing command (left): %s\n", left->arguments[0]);
        }

        // connect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 child1 stdout");
            _exit(1);
        }
        // close unused ends
        close(pipefd[0]);
        close(pipefd[1]);

        // left side may have input redirection, but NOT output (we checked)
        handle_redirection(left);

        // exec left cmd
        if (execute(left) == -1)
        {
            perror("execvp left");
            _exit(1);
        }
    }
    addProcess(process_list, left, cpid1);

    // ----- SECOND CHILD (right side of |) -----
    cpid2 = fork();
    if (cpid2 < 0)
    {
        perror("fork");
        // parent: close pipe ends and reap first child
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(cpid1, NULL, 0);
        return;
    }
    if (cpid2 == 0)
    {
        // CHILD 2
        if (!left->blocking)
        {
            setpgid(0, 0);
        }

        if (debug_mode)
        {
            fprintf(stderr, "PID: %d (child2)\n", getpid());
            fprintf(stderr, "Executing command (right): %s\n", right->arguments[0]);
        }

        // connect stdin to pipe read end
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2 child2 stdin");
            _exit(1);
        }
        // close unused ends
        close(pipefd[1]);
        close(pipefd[0]);

        // right side may have output redirection, but NOT input (we checked)
        handle_redirection(right);

        // exec right cmd
        if (execute(right) == -1)
        {
            perror("execvp right");
            _exit(1);
        }
    }
    addProcess(process_list, right, cpid2);

    // Most important thing: close both ends in parent
    close(pipefd[0]);
    close(pipefd[1]);

    if (left->blocking)
    {
        // Wait for BOTH children, in any order or in order – assignment says order of execution, so:
        waitpid(cpid1, NULL, 0);
        waitpid(cpid2, NULL, 0);
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

    struct process *process_list = NULL;

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

            if (strcmp(userinput, "procs\n") == 0)
            {
                printProcessList(&process_list);
                continue;
            }

            cmdLine *cmd = parseCmdLines(userinput);

            // cd handling
            if (strcmp(cmd->arguments[0], "cd") == 0)
            {
                if (chdir(cmd->arguments[1]) != 0)
                    fprintf(stderr, "cd failed\n");

                freeCmdLines(cmd);
                continue;
            }

            // process signal commands: zzzz, kuku, blast
            if (handle_process_commands(cmd, &process_list))
            {
                freeCmdLines(cmd);
                continue;
            }

            if (cmd->next != NULL)
            {
                // ------- PIPELINE CASE -------

                // Only support a single pipe: cmd | cmd
                if (cmd->next->next != NULL)
                {
                    fprintf(stderr, "Error: only a single pipe is supported\n");
                    freeCmdLines(cmd);
                    continue;
                }
                cmdLine *left = cmd;
                cmdLine *right = cmd->next;

                // Check illegal redirections
                if (left->outputRedirect != NULL)
                {
                    fprintf(stderr, "Error: cannot redirect output of left-hand side of a pipeline\n");
                    freeCmdLines(cmd);
                    continue;
                }

                if (right->inputRedirect != NULL)
                {
                    fprintf(stderr, "Error: cannot redirect input of right-hand side of a pipeline\n");
                    freeCmdLines(cmd);
                    continue;
                }
                // Valid pipeline: execute it
                execute_pipeline(left, &process_list);

                // Parent: free the whole cmd list
                // freeCmdLines(cmd);
                continue;
            }

            int child_pid = fork();

            // if we're the child process
            if (child_pid == 0)
            {
                if (!cmd->blocking)
                {
                    setpgid(0, 0);
                }
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
                // add to process list
                addProcess(&process_list, cmd, child_pid);
                if (cmd->blocking)
                    waitpid(child_pid, NULL, 0);
            }

            // freeCmdLines(cmd);
        }
    }

    freeProcessList(&process_list);
    return 0;
}
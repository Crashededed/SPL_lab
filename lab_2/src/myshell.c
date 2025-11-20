#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include "LineParser.h"

//ref
int execute(cmdLine *pCmdLine)
{
    return execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

int main(int argc, char **argv)
{

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
            if (execute(cmd) == -1)
            {
                freeCmdLines(cmd);
                perror("execv failed\n");
                return 1;
            }

            freeCmdLines(cmd);
        }
    }

    return 0;
}
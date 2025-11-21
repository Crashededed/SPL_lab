#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    // child
    if (pid == 0) {
        close(fd[0]);
        write(fd[1], argv[1], strlen(argv[1]) + 1);
        close(fd[1]);
        exit(0);
    }
    // parent
    else {
        close(fd[1]);
        char buffer[256];
        ssize_t n = read(fd[0], buffer, sizeof(buffer));
        if (n > 0) {
            printf("Parent received: %s\n", buffer);
        } else {
            printf("Parent received no data.\n");
        }
        close(fd[0]);
    }

    return 0;
}

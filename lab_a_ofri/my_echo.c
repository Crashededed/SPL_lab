#include <stdio.h>

//ref: W3schools
int main(int argc, char* argv[]) {
    // Prining each argument
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }

    return 0;
}
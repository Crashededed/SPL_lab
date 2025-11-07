#include <stdio.h>

int main(int argc, char* argv[]) {

    // Printing the coundt of arguments
  	printf("The value of argc is %d\n", argc);

    // Prining each argument
    for (int i = 0; i < argc; i++) {
        printf("%s \n", argv[i]);
    }

    return 0;
}
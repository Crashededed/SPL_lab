#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    /* TODO: Complete during task 2.a */

    for (int i = 0; i < array_length; i++)
    {
        //https://www.w3schools.com/c/c_functions_pointers.php
        mapped_array[i] = (*f)(array[i]);
    }

    return mapped_array;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c)
{
    return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c, otherwise, the dot ('.') character.
This is followed by a space character, and then the value of c in a hexadecimal.
Finally, a new line character is printed. After printing, cprt returns the value of c unchanged. */
char cxprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
        printf("%c ", c);
    else
        printf(". ");

    printf("%02x\n", c);
    return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */
char encrypt(char c)
{
    if (c >= 0x1F && c <= 0x7E)
        return c + 1;

    return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */
char decrypt(char c)
{
    if (c >= 0x21 && c <= 0x7F)
        return c - 1;

    return c;
}

/* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */
char dprt(char c)
{
    printf("%d\n", c);
    return c;
}

//ref: from moodle
struct fun_desc
{
    char *name;
    char (*fun)(char);
};

//ref: from moodle
struct fun_desc menu[] = {
    {"my_get - enter 4 characters", my_get},
    {"cprt - print saved chars as well as their hexadecimal representation", cxprt},
    {"encrypt - encrypt chars", encrypt},
    {"decrypt - decrypt chars", decrypt},
    {"dprt - print chars as their decimal representation", dprt},
    {NULL, NULL}};

int main(int argc, char **argv)
{
    char input[100];
    //ref: https://www.geeksforgeeks.org/c/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
    char *carray = calloc(5, sizeof(char));

    // calculate bound for functions
    int bound = 0;
    while (menu[bound].name != NULL)
    {
        bound++;
    }

    while (1)
    {
        printf("Select operation from the following menu, or ctrl+d to exit:\n");
        for (int i = 0; i < bound; i++)
            printf(" (%d) - %s\n", i, menu[i].name);

        //ref: https://www.geeksforgeeks.org/c/fgets-function-in-c/
        if (fgets(input, 100, stdin) == NULL)
        {
            printf("Exiting menu.\n");
            break;
        }

        //ref: atoi from moodle
        int choice = atoi(input);

        if (choice >= 0 && choice < bound)
        {
            char *new_arr = map(carray, 5, menu[choice].fun);
            free(carray);
            carray = new_arr;
        }
        else
        {
            printf("Not within bounds. Exiting menu.\n");
            break;
        }
    }
    
    free(carray);
    return 0;
}

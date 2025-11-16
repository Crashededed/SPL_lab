#include <stdio.h>

int password_match(char str[], unsigned char password[])
{
    int i = 0;
    while (1)
    {
        if (str[i] != password[i])
            return 0;

        if (str[i] == '\0')
            return 1;

        i++;
    }
}

char encode(char c)
{
    return c;
}

int main(int argc, char *argv[])
{
    int debug_mode = 1;
    unsigned char password[] = "pass";
    FILE *infile = stdin;
    FILE *outfile = stdout;

    // debug mode handling
    for (int i = 1; i < argc; i++)
    {
        if (debug_mode)
            fprintf(stderr, "%s\n", argv[i]);

        if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0')
            debug_mode = 0;

        if (argv[i][0] == '+' && argv[i][1] == 'D' && password_match(argv[i] + 2, password))
            debug_mode = 1;
    }

    while (1)
    {
        // ref: https://www.geeksforgeeks.org/c/eof-and-feof-in-c/
        // maybe c should be int?
        char c = fgetc(infile);

        if (c == EOF && feof(infile))
        {
            printf("deteced EOF\0");
            break;
        }

        c = encode(c);
        fputc(c, outfile);
    }
    return 0;
}

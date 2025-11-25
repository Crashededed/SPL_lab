#include <stdio.h>

int debug_mode = 1;
char* password = "wordpass";

int password_match(char str[])
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

char *encoding_key = "0";
int sign = 1;
int encoding_index = 0;

char encode(char c)
{
    int encoding_val = sign * (encoding_key[encoding_index] - '0');

    if (c >= '0' && c <= '9')
        c = '0' + ((c - '0' + encoding_val + 10) % 10);

    if (c >= 'A' && c <= 'Z')
        c = 'A' + ((c - 'A' + encoding_val + 26) % 26);

    encoding_index++;
    if (encoding_key[encoding_index] == '\0')
        encoding_index = 0;

    return c;
}

int main(int argc, char *argv[])
{
    FILE *infile = stdin;
    FILE *outfile = stdout;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '+' && argv[i][1] == 'E')
        {
            sign = 1;
            encoding_key = argv[i] + 2;
            if (debug_mode)
                fprintf(stderr, "Encoding with key %s\n", encoding_key);
        }
        if (argv[i][0] == '-' && argv[i][1] == 'E')
        {
            sign = -1;
            encoding_key = argv[i] + 2;
            if (debug_mode)
                fprintf(stderr, "Encoding with key %s\n", encoding_key);
        }
        if (debug_mode)
            fprintf(stderr, "%s\n", argv[i]);

        if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0')
            debug_mode = 0;
        if (argv[i][0] == '+' && argv[i][1] == 'D' && password_match(argv[i] + 2))
            debug_mode = 1;
        if (argv[i][0] == '-' && argv[i][1] == 'i'){
            infile = fopen(argv[i] + 2, "r");
            if (infile == NULL){
                fprintf(stderr, "Error opening input file: %s\n", argv[i] + 2);
                return 1;
            }
            if (debug_mode)
                fprintf(stderr, "Input file opened: %s\n", argv[i] + 2);
        }
        if (argv[i][0] == '-' && argv[i][1] == 'o'){
            outfile = fopen(argv[i] + 2, "w");
            if (outfile == NULL){
                fprintf(stderr, "Error opening output file: %s\n", argv[i] + 2);
                return 1;
            }
            if (debug_mode)
                fprintf(stderr, "Output file opened: %s\n", argv[i] + 2);
        }
    }

    while (1)
    {
        int c = fgetc(infile);
        if (c == EOF){
            break;
        }
        c = encode(c);
        fputc(c, outfile);
    }
    return 0;
}

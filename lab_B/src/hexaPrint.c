// haxaPrint.c
// Read a binary file given as argv[1] and print its bytes as hex to stdout.
// Usage: haxaPrint <file>

#include <stdio.h>
#include <stdlib.h>

int X = 16;
void PrintHex(unsigned char *buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i]);
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 2;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        perror("fopen failed");
        return 1;
    }
    unsigned char inbuf[X];
    int bytes_read = 0;
    while ((bytes_read = fread(inbuf, sizeof(unsigned char), X, f)))
    {
        PrintHex(inbuf, bytes_read);
    }
    printf("\n");
    fclose(f);
    return 0;
}
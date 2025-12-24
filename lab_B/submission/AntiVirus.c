#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 256

// struct taken from lab 1
struct fun_desc
{
    char *name;
    void (*fun)(void)
};

// remove padding to read into struct correctly
#pragma pack(push, 1)
typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;
#pragma pack(pop)

typedef struct link link;
struct link
{
    link *nextVirus;
    virus *vir;
};

void PrintHex(unsigned char *buffer, int length, FILE *file)
{
    for (int i = 0; i < length; i++)
    {
        fprintf(file, "%02X ", buffer[i]);
    }
}

void printSignature(virus *virus, FILE *file)
{
    fprintf(file, "Virus name: %s\nVirus signatue length: %d\nVirus signature:\n", virus->virusName, virus->SigSize);
    PrintHex(virus->sig, virus->SigSize, file);
    fprintf(file, "\n");
}

void list_print(link *virus_list, FILE *file)
{
    link *current_link = virus_list;
    while (current_link)
    {
        printSignature(current_link->vir, file);
        printf("\n");
        current_link = current_link->nextVirus;
    }
}

link *list_append(link *virus_list, virus *data)
{
    link *new_link = malloc(sizeof(link));
    if (!data)
    {
        free(new_link);
        return virus_list;
    }

    new_link->vir = data;
    new_link->nextVirus = virus_list; // Point to current head
    return new_link; // New link becomes the new head
}

void list_free(link *virus_list)
{
    link *current = virus_list;
    while (current)
    {
        link *next = current->nextVirus;
        if (current->vir)
        {
            free(current->vir->sig);
            free(current->vir);
        }
        free(current);
        current = next;
    }
}

link *SignaturesList = NULL;
int is_big_endian = 0;

virus *readSignature(FILE *file)
{
    virus *virus = malloc(sizeof(struct virus));

    if (fread(virus, 1, 18, file) != 18)
    {

        free(virus);
        if (!feof(file))
            perror("Couldnt read fully virus header");

        return NULL;
    }

    if (is_big_endian)
    {
        unsigned short s = virus->SigSize;
        virus->SigSize = (s >> 8) | (s << 8); // swap byte order
    }

    virus->sig = (unsigned char *)malloc(virus->SigSize);

    if (fread(virus->sig, 1, virus->SigSize, file) != virus->SigSize)
    {
        perror("Couldnt read fully virus signature");
        free(virus->sig);
        free(virus);
        return NULL;
    }

    return virus;
}

void LoadSignatures()
{
    char filename[MAX_FILENAME] = {0};

    printf("Enter a virus signatures file name:\n");
    fgets(filename, MAX_FILENAME, stdin);

    filename[strcspn(filename, "\n")] = '\0';
    FILE *file = fopen(filename, "rb");

    if (!file)
    {
        perror("Could not open signatures file");
        return;
    }
    char magic_number[5] = {0};

    if (fread(magic_number, 1, 4, file) != 4)
    {
        perror("couldn't read file magic number");
        fclose(file);
        return;
    }
    if (strcmp(magic_number, "VIRL") == 0)
    {
        is_big_endian = 0;
    }
    else if (strcmp(magic_number, "VIRB") == 0)
    {
        is_big_endian = 1;
    }
    else
    {
        printf("incorrect magic number: %s for file %s\n", magic_number, filename);
        fclose(file);
        return;
    }

    list_free(SignaturesList);
    SignaturesList = NULL;

    while (1)
    {
        virus *next_virus = readSignature(file);
        if (!next_virus)
            break;

        SignaturesList = list_append(SignaturesList, next_virus);
    }
    fclose(file);
}

void PrintSignatures()
{
    if (!SignaturesList)
    {
        printf("No signatures\n");
        return;
    }

    char filename[MAX_FILENAME] = {0};
    printf("Press enter for console output or enter file path:\n");
    fgets(filename, MAX_FILENAME, stdin);

    filename[strcspn(filename, "\n")] = '\0';
    if ('\0' == filename[0])
    {
        list_print(SignaturesList, stdout);
    }
    else
    {
        FILE *file = fopen(filename, "wb");
        if (!file)
        {
            perror("Could not open file for writing");
            return;
        }
        list_print(SignaturesList, file);
        fclose(file);
    }
}

void detect_virus()
{
    char filename[MAX_FILENAME] = {0};

    printf("Enter a file name to scan for viruses:\n");
    fgets(filename, MAX_FILENAME, stdin);
    filename[strcspn(filename, "\n")] = '\0';

    unsigned char *buffer = (unsigned char *)malloc(10000);
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Could not open file to scan");
        free(buffer);
        return;
    }

    int filesize = fread(buffer, 1, 10000, file);
    fclose(file);

    for (int i = 0; i < filesize; i++)
    {
        link *current = SignaturesList;
        while (current)
        {
            if (i + current->vir->SigSize > filesize)
            {
                current = current->nextVirus;
                continue;
            }
            if (!memcmp(buffer + i, current->vir->sig, current->vir->SigSize))
            {
                printf("virus location: %d\nvirus name: %s\nvirus signature size: %d\n", i, current->vir->virusName, current->vir->SigSize);
            }
            current = current->nextVirus;
        }
    }
    free(buffer);
}

void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE *file = fopen(fileName, "r+b");
    if (!file)
    {
        perror("Could not open file to neutralize virus");
        return;
    }

    fseek(file, signatureOffset, SEEK_SET);
    unsigned char ret = 0xC3; // RET instruction in x86
    fwrite(&ret, 1, 1, file);

    fclose(file);
}

void fixFile()
{
    char filename[MAX_FILENAME] = {0};

    printf("Enter a file name to scan for viruses and remove them:\n");
    fgets(filename, MAX_FILENAME, stdin);
    filename[strcspn(filename, "\n")] = '\0';

    unsigned char *buffer = (unsigned char *)malloc(10000);
    FILE *file = fopen(filename, "rb");

    if (!file)
    {
        perror("Could not open file to fix");
        free(buffer);
        return;
    }
    
    int filesize = fread(buffer, 1, 10000, file);

    fclose(file);

    for (int i = 0; i < filesize; i++)
    {
        link *current = SignaturesList;
        while (current)
        {
            if (i + current->vir->SigSize > filesize)
            {
                current = current->nextVirus;
                continue;
            }
            if (!memcmp(buffer + i, current->vir->sig, current->vir->SigSize))
            {
                printf("virus location: %d\nvirus name: %s\nvirus signature size: %d\n", i, current->vir->virusName, current->vir->SigSize);
                neutralize_virus(filename, i);
            }
            current = current->nextVirus;
        }
    }
    free(buffer);
}

void Quit()
{
    list_free(SignaturesList);
    exit(0);
}

void stub()
{
    printf("Not implemented yet\n");
}

struct fun_desc menu[] =
    {{"Load signatures", LoadSignatures},
     {"Print signatures", PrintSignatures},
     {"Detect Viruses", detect_virus},
     {"Fix file", fixFile},
     {"AI analysis of file", stub},
     {"Quit", Quit},
     {NULL, NULL}};

int main(int argc, char **argv)
{
    int menu_size = 0;
    while (menu[menu_size].name != NULL)
        menu_size++;

    while (1)
    {
        char input[16];
        int choice;

        printf("Select one of the above options:\n");
        for (int i = 0; i < menu_size; i++)
        {
            printf("%d. %s\n", i, menu[i].name);
        }

        printf("Choose a function by it's number; Only the first character of the input will be read:\n");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input.\n");
            continue;
        }

        // Parse input
        if (sscanf(input, "%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        //  Validate choice
        if (choice < 0 || choice >= menu_size)
        {
            printf("Choice out of bounds. Try again.\n");
            continue;
        }
        menu[choice].fun();
    }
    list_free(SignaturesList);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fun_desc
{
    char *name;
    void (*fun)(void);
};

char decimal = 0;
char debug = 0;
char file_name[128];
int unit_size = 1;
unsigned char mem_buf[10000];
size_t mem_count;
char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit\n", "%#x\n"};
char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit\n", "%#d\n"};

void ToggleDebugMode(){
    debug = 1 - debug;
    if (debug){ fprintf(stderr, "Debug flag now on\n");;}
    else{ fprintf(stderr, "Debug flag now off\n");;}
}

void SetFileName(){
    char input[128] = {0};
    printf("Enter a file name:\n");
    fgets(input, 128, stdin);
    input[strcspn(input, "\n")] = '\0';
    if (input[0] != 0)
        strcpy(file_name, input);
    if(debug)
        fprintf(stderr, "Debug: file name set to %s\n", file_name);
}

void SetUnitSize(){
    char input[128] = {0};
    printf("Enter unit size:\n");
    fgets(input, 128, stdin);
    if ((input[0] == '1' || input[0] == '2' || input[0] == '4') && input[1] == '\n'){
        unit_size = input[0] - '0';
        if(debug)
            fprintf(stderr, "Debug: set size to %u\n", unit_size);
    }
    else {perror("Not a valid unit size; Valid unit sizes are 1, 2, 4\n");}
}

void LoadIntoMemory(){
    char input[128] = {0};
    unsigned int location, length;
    if(file_name[0] == 0){
        perror("no file name to open\n");
        return;
    }
    FILE* file = fopen(file_name, "rb");
    if(!file){
        fprintf(stderr, "file %s not found\n", file_name);
        return;
    }
    printf("Please enter <location> <length>\n");
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "Error reading input\n");
        fclose(file);
        return;
    }
    if (sscanf(input, "%x %u", &location, &length) != 2) {
        fprintf(stderr, "Error: invalid input format\n");
        fclose(file);
        return;
    }
    if (debug) {
        fprintf(stderr, "Debug: file_name='%s', location=0x%x, length=%u\n", file_name, location, length);
    }
    if (length * unit_size > sizeof(mem_buf)) {
        fprintf(stderr, "Error: requested size too large\n");
        fclose(file);
        return;
    }
    if (fseek(file, location, SEEK_SET) != 0) {
        perror("Error seeking in file");
        fclose(file);
        return;
    }
    mem_count = fread(mem_buf, unit_size, length, file);
    printf("Loaded %zu units into memory\n", mem_count);
    fclose(file);
}

void ToggleDisplayMode(){
    decimal = 1 - decimal;
    if (decimal){ printf("Decimal display flag now on, decimal representation\n");}
    else{ printf("Decimal display flag now off, hexadecimal representation\n");}
}

void MemoryDisplay(){
    char input[128] = {0};
    unsigned char* start;
    unsigned int address, u;
    printf("Please enter <address> <u>\n");
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "Error reading input\n");
        return;
    }
    if (sscanf(input, "%x %u", &address, &u) != 2) {
        fprintf(stderr, "Error: invalid input format\n");
        return;
    }
    if (address == 0)
        start = mem_buf;
    else {
        if (address + u * unit_size > mem_count * unit_size) {
            fprintf(stderr, "Error: out of bounds\n");
            return;
        }
        start = mem_buf + address;
    }
    
    for (int i = 0; i < u; i++) {
        unsigned int val = 0;
        memcpy(&val, start + i * unit_size, unit_size);
        if (decimal) {
            printf(dec_formats[unit_size - 1], val);
        } else {
            printf(hex_formats[unit_size - 1], val);
        }
    }
}

void SaveIntoFile(){
    FILE* file;
    char input[128];
    unsigned int source_addr;
    unsigned int target_loc;
    unsigned int length;
    unsigned char* source;
    long file_size;

    if (file_name[0] == '\0') {
        fprintf(stderr, "Error: file name is not set\n");
        return;
    }
    file = fopen(file_name, "r+b");
    if (!file) {
        perror("Error opening file");
        return;
    }

    printf("Please enter <source-address> <target-location> <length>\n");
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "Error reading input\n");
        fclose(file);
        return;
    }
    if (sscanf(input, "%x %x %u",
               &source_addr, &target_loc, &length) != 3) {
        fprintf(stderr, "Error: invalid input format\n");
        fclose(file);
        return;
    }
    if (debug) {
        fprintf(stderr,
                "Debug: source-address=0x%x, target-location=0x%x, length=%u\n",
                source_addr, target_loc, length);
    }
    if (source_addr == 0)
        source = mem_buf;
    else
        source = (unsigned char*)source_addr;
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    if (target_loc + length * unit_size > file_size) {
        fprintf(stderr, "Error: write exceeds file size\n");
        fclose(file);
        return;
    }
    if (fseek(file, target_loc, SEEK_SET) != 0) {
        perror("Error seeking in file");
        fclose(file);
        return;
    }
    fwrite(source, unit_size, length, file);
    fclose(file);
}

void MemoryModify(){
    unsigned int location, val;
    char input[128];
    printf("Please enter <location> <val>\n");
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "Error reading input\n");
        return;
    }
    if (sscanf(input, "%x %x", &location, &val) != 2) {
        fprintf(stderr, "Error: invalid input format\n");
        return;
    }
    if(debug)
        fprintf(stderr, "DEBUG: location: 0x%x, val: 0x%x\n", location, val);
    if (location + unit_size > mem_count * unit_size) {
        fprintf(stderr, "Error: location outside loaded memory\n");
        return;
    }
    void* target = (char*)mem_buf + location;
    if (unit_size == 1) {
        unsigned char* p = (unsigned char*)target;
        *p = (unsigned char)val;
    } else if (unit_size == 2) {
        unsigned short* p = (unsigned short*)target;
        *p = (unsigned short)val;
    } else if (unit_size == 4) {
        unsigned int* p = (unsigned int*)target;
        *p = (unsigned int)val;
    }
}

void quit(){
    if (debug)
        fprintf(stderr, "quitting\n");
    exit(0);
}

void stub()
{
    printf("Not implemented yet\n");
}

struct fun_desc menu[] =
    {{"Toggle Debug Mode", ToggleDebugMode},
     {"Set File Name", SetFileName},
     {"Set Unit Size", SetUnitSize},
     {"Load Into Memory", LoadIntoMemory},
     {"Toggle Display Mode", ToggleDisplayMode},
     {"Memory Display", MemoryDisplay},
     {"Save Into File", SaveIntoFile},
     {"Memory Modify", MemoryModify},
     {"Quit", quit},
     {NULL, NULL}};

int main(int argc, char **argv){
    int menu_size = 0;
    while (menu[menu_size].name != NULL)
        menu_size++;

    while (1)
    {
        char input[16];
        int choice;
        if(debug)
            fprintf(stderr, "Debug: unit_size=%d, file_name=%s, mem_count=%zu\n", unit_size, file_name, mem_count);

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
}
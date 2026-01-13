#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#define FILE_CAPACITY 2

typedef struct
{
    char name[100];  // ELF file name
    int fd;          // file descriptor
    void *map_start; // mapped address
    off_t size;      // file size
    int used;        // 0 = empty, 1 = occupied
} ELF_Info;

char debug = 0;
ELF_Info files[FILE_CAPACITY];

void examine_elf_file()
{
    char filename[100];

    // Find a free slot
    int index = -1;
    for (int i = 0; i < FILE_CAPACITY; i++)
    {
        if (!files[i].used)
        {
            index = i;
            break;
        }
    }

    // check if there is a free slot, otherwise print error and return
    if (index == -1)
    {
        printf("Error: Max number of files (2) already open.\n");
        return;
    }

    printf("Enter ELF file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL)
    {
        printf("Error reading input.\n");
        return;
    }
    // Remove the trailing newline
    filename[strcspn(filename, "\n")] = 0;

    // Open File
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    // get file size
    struct stat st;
    if (fstat(fd, &st) != 0)
    {
        perror("fstat");
        close(fd);
        return;
    }

    // Map file into memory
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("mmap failed");
        close(fd);
        return;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map;

    printf("Bytes 1,2,3 of magic number: %c %c %c\n",
           ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);

    // check magic number
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        printf("Error: Not a valid ELF file.\n");
        munmap(map, st.st_size);
        close(fd);
        return;
    }

    // store info
    strcpy(files[index].name, filename);
    files[index].used = 1;
    files[index].fd = fd;
    files[index].map_start = map;
    files[index].size = st.st_size;

    // print info
    printf("Data Encoding: %s\n",
           ehdr->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" : ehdr->e_ident[EI_DATA] == ELFDATA2MSB ? "2's complement, big endian"
                                                                                                                           : "Unknown");

    printf("Entry point: 0x%x\n", ehdr->e_entry);

    printf("Section header table offset: %d\n", ehdr->e_shoff);
    printf("Num of section header entries: %d\n", ehdr->e_shnum);
    printf("Size of section header entries: %d\n", ehdr->e_shentsize);

    printf("Program header table offset: %d\n", ehdr->e_phoff);
    printf("Num of program header entries: %d\n", ehdr->e_phnum);
    printf("Size of program header entries: %d\n", ehdr->e_phentsize);

    if (debug)
    {
        printf("\n[DEBUG] File mapped at address: %p\n", map);
        printf("stored at index %d\n", index);
    }
}

void print_section_names()
{
    for (size_t k = 0; k < FILE_CAPACITY; k++)
    {
        if (files[k].used)
        {
            printf("File %s\n", files[k].name);
            Elf32_Ehdr *ehdr = (Elf32_Ehdr *)files[k].map_start;                         // ELF header
            Elf32_Shdr *shdr_table = (Elf32_Shdr *)(files[k].map_start + ehdr->e_shoff); // Section header table

            if (ehdr->e_shstrndx == SHN_UNDEF)
            {
                printf("Error: No section header string table found.\n");
                continue;
            }

            Elf32_Shdr *sh_strtab_header = &shdr_table[ehdr->e_shstrndx];                 // header for the section header string table
            char *sh_strtab = (char *)(files[k].map_start + sh_strtab_header->sh_offset); // pointer to section header string table

            if (debug)
            {
                printf("[DEBUG] Section Header Table offset: %d\n", ehdr->e_shoff);
                printf("[DEBUG] String Table index (shstrndx): %d\n", ehdr->e_shstrndx);
                printf("[DEBUG] String Table offset: %d\n", sh_strtab_header->sh_offset);
            }

            for (int i = 0; i < ehdr->e_shnum; i++)
            {
                // Get the name string using the index (sh_name) from the header
                // index = offset into the string table
                char *name = sh_strtab + shdr_table[i].sh_name;

                // Format: [index] section_name section_address section_offset section_size section_type
                printf("[%2d] %-20s %08x %06x %06x %02d\n",
                       i,
                       name,
                       shdr_table[i].sh_addr,
                       shdr_table[i].sh_offset,
                       shdr_table[i].sh_size,
                       shdr_table[i].sh_type);
            }
        }
    }
}

void print_symbols()
{
    for (int k = 0; k < FILE_CAPACITY; k++)
    {
        if (files[k].used)
        {
            printf("File %s\n", files[k].name);

            Elf32_Ehdr *ehdr = (Elf32_Ehdr *)files[k].map_start;
            Elf32_Shdr *shdr_table = (Elf32_Shdr *)(files[k].map_start + ehdr->e_shoff); // Section header table

            if (ehdr->e_shstrndx == SHN_UNDEF)
            {
                printf("Error: No section header string table found.\n");
                continue;
            }

            Elf32_Shdr *sh_strtab_header = &shdr_table[ehdr->e_shstrndx];                 // header for the section header string table
            char *sh_strtab = (char *)(files[k].map_start + sh_strtab_header->sh_offset); // pointer to section header string table

            for (int i = 0; i < ehdr->e_shnum; i++) // iterate over section headers until symbol table is found
            {
                if (shdr_table[i].sh_type == SHT_SYMTAB)
                {

                    Elf32_Sym *sym_table = (Elf32_Sym *)(files[k].map_start + shdr_table[i].sh_offset); // symbol table
                    int num_symbols = shdr_table[i].sh_size / sizeof(Elf32_Sym);

                    int symbol_strtab_index = shdr_table[i].sh_link;                            // string table offset for symbols
                    Elf32_Shdr *strtab_hdr = &shdr_table[symbol_strtab_index];                  // string table section header
                    char *symbol_strtab = (char *)(files[k].map_start + strtab_hdr->sh_offset); // pointer to string table containing symbol names

                    if (debug)
                    {
                        printf("[DEBUG] Symbol Table found at section index %d\n", i);
                        printf("[DEBUG] Num symbols: %d\n", num_symbols);
                        printf("[DEBUG] Symbol String Table index: %d\n", symbol_strtab_index);
                    }

                    for (int j = 0; j < num_symbols; j++)
                    {
                        Elf32_Sym *sym = &sym_table[j];

                        // Get Symbol Name
                        char *sym_name = symbol_strtab + sym->st_name;

                        // Get Section Name logic
                        char *section_name;
                        if (sym->st_shndx == SHN_UNDEF)
                            section_name = "UND";

                        else if (sym->st_shndx == SHN_ABS)
                            section_name = "ABS";

                        else if (sym->st_shndx == SHN_COMMON)
                            section_name = "COM";

                        else if (sym->st_shndx < ehdr->e_shnum) // valid section index
                            section_name = sh_strtab + shdr_table[sym->st_shndx].sh_name;

                        else
                            section_name = "???";

                        // Print Format: [index] value section_index section_name symbol_name
                        printf("[%2d] %08x %5d %-20s %s\n",
                               j,
                               sym->st_value,
                               sym->st_shndx,
                               section_name,
                               sym_name);
                    }
                }
            }
        }
    }
}

// Helper function to convert relocation type number to string
char *rel_type_to_string(int type)
{
    switch (type)
    {
    case 0:
        return "R_386_NONE";
    case 1:
        return "R_386_32";
    case 2:
        return "R_386_PC32";
    case 3:
        return "R_386_GOT32";
    case 4:
        return "R_386_PLT32";
    case 5:
        return "R_386_COPY";
    case 6:
        return "R_386_GLOB_DAT";
    case 7:
        return "R_386_JMP_SLOT";
    case 8:
        return "R_386_RELATIVE";
    case 9:
        return "R_386_GOTOFF";
    case 10:
        return "R_386_GOTPC";
    default:
        return "UNKNOWN";
    }
}

void print_relocations()
{
    for (int k = 0; k < FILE_CAPACITY; k++)
    {
        if (files[k].used)
        {
            printf("File %s relocations\n", files[k].name);

            Elf32_Ehdr *ehdr = (Elf32_Ehdr *)files[k].map_start;
            Elf32_Shdr *shdr_table = (Elf32_Shdr *)(files[k].map_start + ehdr->e_shoff); // Section header table

            if (ehdr->e_shstrndx == SHN_UNDEF)
            {
                printf("Error: No section header string table found.\n");
                continue;
            }

            Elf32_Shdr *sh_strtab_header = &shdr_table[ehdr->e_shstrndx];                 // header for the section header string table
            char *sh_strtab = (char *)(files[k].map_start + sh_strtab_header->sh_offset); // pointer to section header string table

            int has_relocations = 0;

            for (int i = 0; i < ehdr->e_shnum; i++) // iterate over section headers to find relocation sections
            {
                if (shdr_table[i].sh_type == SHT_REL)
                {
                    has_relocations = 1;

                    // Define the table of relocations
                    Elf32_Rel *rel_table = (Elf32_Rel *)(files[k].map_start + shdr_table[i].sh_offset);
                    int num_entries = shdr_table[i].sh_size / sizeof(Elf32_Rel);

                    // find the Symbol Table for those Relocations
                    int sym_tab_index = shdr_table[i].sh_link;
                    Elf32_Shdr *sym_tab_header = &shdr_table[sym_tab_index];
                    Elf32_Sym *sym_table = (Elf32_Sym *)(files[k].map_start + sym_tab_header->sh_offset);

                    // find the String Table for those Symbols
                    int str_tab_index = sym_tab_header->sh_link;
                    Elf32_Shdr *str_tab_header = &shdr_table[str_tab_index];
                    char *str_tab = (char *)(files[k].map_start + str_tab_header->sh_offset);

                    if (debug)
                    {
                        printf("Relocation section '%s' at offset 0x%x contains %d entries:\n",
                               sh_strtab + shdr_table[i].sh_name,
                               shdr_table[i].sh_offset,
                               num_entries);
                    }

                    // Loop through Relocation Entries
                    for (int j = 0; j < num_entries; j++)
                    {
                        Elf32_Rel *rel = &rel_table[j];

                        int sym_index = ELF32_R_SYM(rel->r_info);
                        int type = ELF32_R_TYPE(rel->r_info);

                        // Retrieve Symbol Name
                        char *sym_name = "";
                        int sym_value = 0;
                        if (sym_index != 0) // if not the undefined symbol
                        {
                            Elf32_Sym *sym = &sym_table[sym_index];
                            sym_name = str_tab + sym->st_name;
                            sym_value = sym->st_value;
                        }

                        printf("[%2d] %08x %-20s %08x %-10s\n",
                               j,
                               rel->r_offset,
                               sym_name,
                               sym_value,
                               rel_type_to_string(type));
                    }
                }
            }
            if (!has_relocations)
            {
                printf("No relocations\n");
            }
        }
    }
}

void CheckMerge()
{
    // 1. Verify two files are open
    if (!files[0].used || !files[1].used)
    {
        printf("Error: Two ELF files must be opened for merge.\n");
        return;
    }

    int found_error = 0;

    // Extract section headers from both files
    ELF_Info *f1 = &files[0];
    ELF_Info *f2 = &files[1];

    Elf32_Ehdr *ehdr1 = (Elf32_Ehdr *)f1->map_start;
    Elf32_Shdr *shdr1 = (Elf32_Shdr *)(f1->map_start + ehdr1->e_shoff);

    Elf32_Ehdr *ehdr2 = (Elf32_Ehdr *)f2->map_start;
    Elf32_Shdr *shdr2 = (Elf32_Shdr *)(f2->map_start + ehdr2->e_shoff);

    // Locate symbol tables and string tables in both files
    Elf32_Sym *symtab1 = NULL;
    char *strtab1 = NULL;
    int num_syms1 = 0;

    Elf32_Sym *symtab2 = NULL;
    char *strtab2 = NULL;
    int num_syms2 = 0;

    // Find Symtab 1
    for (int i = 0; i < ehdr1->e_shnum; i++)
    {
        if (shdr1[i].sh_type == SHT_SYMTAB)
        {
            symtab1 = (Elf32_Sym *)(f1->map_start + shdr1[i].sh_offset);
            num_syms1 = shdr1[i].sh_size / sizeof(Elf32_Sym);
            strtab1 = (char *)(f1->map_start + shdr1[shdr1[i].sh_link].sh_offset);
            break;
        }
    }

    // Find Symtab 2
    for (int i = 0; i < ehdr2->e_shnum; i++)
    {
        if (shdr2[i].sh_type == SHT_SYMTAB)
        {
            symtab2 = (Elf32_Sym *)(f2->map_start + shdr2[i].sh_offset);
            num_syms2 = shdr2[i].sh_size / sizeof(Elf32_Sym);
            strtab2 = (char *)(f2->map_start + shdr2[shdr2[i].sh_link].sh_offset);
            break;
        }
    }

    if (!symtab1 || !symtab2)
    {
        printf("Error: Missing symbol table in one of the files.\n");
        return;
    }

    // Compare symbols from File 1 against File 2
    for (int i = 1; i < num_syms1; i++) // skip first symbol
    {
        Elf32_Sym *sym1 = &symtab1[i];
        char *name1 = strtab1 + sym1->st_name;

        // Skip empty names
        if (strlen(name1) == 0)
            continue;

        // Search for this symbol in File 2
        Elf32_Sym *sym2 = NULL;
        for (int j = 1; j < num_syms2; j++)
        {
            char *name2 = strtab2 + symtab2[j].st_name;
            if (strcmp(name1, name2) == 0)
            {
                sym2 = &symtab2[j];
                break;
            }
        }

        // Check for undefined symbols
        if (sym1->st_shndx == SHN_UNDEF)
        {
            // Error if Not found OR Found but also Undefined
            if (sym2 == NULL || sym2->st_shndx == SHN_UNDEF)
            {
                found_error = 1;
                printf("Error: Symbol %s undefined\n", name1);
            }
        }
        // Check for multiply defined symbols
        else
        {
            // Error if Found and Defined
            if (sym2 != NULL && sym2->st_shndx != SHN_UNDEF)
            {
                found_error = 1;
                printf("Error: Symbol %s multiply defined\n", name1);
            }
        }
    }

    if (!found_error)
    {
        printf("No merge conflicts detected between %s and %s\n", f1->name, f2->name);
    }
}

// Helper to find a section header in a file by its name
Elf32_Shdr *get_section_by_name(ELF_Info *file, const char *target_name)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file->map_start;
    Elf32_Shdr *shdr_table = (Elf32_Shdr *)(file->map_start + ehdr->e_shoff);

    // Get the String Table for section names
    if (ehdr->e_shstrndx == SHN_UNDEF)
        return NULL;
    char *sh_strtab = (char *)(file->map_start + shdr_table[ehdr->e_shstrndx].sh_offset);

    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        char *name = sh_strtab + shdr_table[i].sh_name;
        if (strcmp(name, target_name) == 0)
        {
            return &shdr_table[i];
        }
    }
    return NULL;
}

void merge_elf_files()
{
    if (!files[0].used || !files[1].used)
    {
        printf("Error: Two ELF files must be opened for merge.\n");
        return;
    }

    ELF_Info *f1 = &files[0];
    ELF_Info *f2 = &files[1];

    // Open/create output file
    int out_fd = open("out.ro", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0)
    {
        perror("open out.ro");
        return;
    }

    // prepare ELF Header for new file based on f1's header
    Elf32_Ehdr *ehdr1 = (Elf32_Ehdr *)f1->map_start;
    Elf32_Ehdr out_ehdr = *ehdr1; // Make a local copy to modify later

    // Write header placeholder into file
    if (write(out_fd, &out_ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("write header");
        close(out_fd);
        return;
    }

    // prepare Section Header Table for new file based on f1's table
    Elf32_Shdr *shdr1 = (Elf32_Shdr *)(f1->map_start + ehdr1->e_shoff);
    int sh_table_size = ehdr1->e_shnum * sizeof(Elf32_Shdr);

    // Allocate memory for the new table so we can modify offsets/sizes
    Elf32_Shdr *out_shdrs = malloc(sh_table_size);
    memcpy(out_shdrs, shdr1, sh_table_size);

    // Get Section Name String Table from F1 to identify sections
    char *sh_strtab1 = (char *)(f1->map_start + shdr1[ehdr1->e_shstrndx].sh_offset);

    // loop through sections in f1 and merge as needed
    for (int i = 0; i < ehdr1->e_shnum; i++)
    {
        char *sec_name = sh_strtab1 + shdr1[i].sh_name;

        // Update offset in new table to current file position
        off_t current_offset = lseek(out_fd, 0, SEEK_CUR);
        out_shdrs[i].sh_offset = current_offset;

        // write section data from f1 into out.ro
        if (shdr1[i].sh_type != SHT_NOBITS)
        {
            if (write(out_fd, f1->map_start + shdr1[i].sh_offset, shdr1[i].sh_size) != shdr1[i].sh_size)
            {
                perror("write section f1");
                free(out_shdrs);
                close(out_fd);
                return;
            }
        }

        // check if this section needs to be merged
        if (strcmp(sec_name, ".text") == 0 ||
            strcmp(sec_name, ".data") == 0 ||
            strcmp(sec_name, ".rodata") == 0)
        {

            // Find corresponding section in f2
            Elf32_Shdr *shdr2 = get_section_by_name(f2, sec_name);
            if (shdr2)
            {
                // Write from corresponding section in f2
                if (write(out_fd, f2->map_start + shdr2->sh_offset, shdr2->sh_size) != shdr2->sh_size)
                {
                    perror("write section f2");
                    free(out_shdrs);
                    close(out_fd);
                    return;
                }

                // update size in new section headers
                out_shdrs[i].sh_size += shdr2->sh_size;

                if (debug)
                {
                    printf("[DEBUG] Merging %s: F1 Size %d + F2 Size %d = %d\n",
                           sec_name, shdr1[i].sh_size, shdr2->sh_size, out_shdrs[i].sh_size);
                }
            }
        }
    }

    // write updated Section Header Table to file
    off_t sh_table_offset = lseek(out_fd, 0, SEEK_CUR);
    if (write(out_fd, out_shdrs, sh_table_size) != sh_table_size)
    {
        perror("write shdr table");
        free(out_shdrs);
        close(out_fd);
        return;
    }

    // update ELF header with new section header offset
    out_ehdr.e_shoff = sh_table_offset;
    lseek(out_fd, 0, SEEK_SET); // Go back to start
    if (write(out_fd, &out_ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("update header");
    }

    printf("Merged ELF file created: out.ro\n");

    // Cleanup
    free(out_shdrs);
    close(out_fd);
}

struct fun_desc
{
    char *name;
    void (*fun)(void);
};

void ToggleDebugMode()
{
    debug = 1 - debug;
    if (debug)
        fprintf(stderr, "Debug flag now on\n");
    else
        fprintf(stderr, "Debug flag now off\n");
}

void quit()
{
    if (debug)
        fprintf(stderr, "quitting\n");
    for (size_t i = 0; i < 2; i++)
    {
        if (files[i].used)
        {
            munmap(files[i].map_start, files[i].size);
            close(files[i].fd);
        }
    }

    exit(0);
}

void stub()
{
    printf("Not implemented yet\n");
}

struct fun_desc menu[] =
    {{"Toggle Debug Mode", ToggleDebugMode},
     {"Examine ELF File", examine_elf_file},
     {"Print Section Names", print_section_names},
     {"Print Symbols", print_symbols},
     {"Print Relocations", print_relocations},
     {"Check Files for Merge", CheckMerge},
     {"Merge ELF File", merge_elf_files},
     {"Quit", quit},
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

        printf("\nSelect one of the above options:\n");
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
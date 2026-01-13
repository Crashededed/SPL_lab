#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

extern int startup(int argc, char **argv, void (*start)());

int foreach_phdr(void* map_start, void (*func)(Elf32_Phdr *, int), int arg){
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr_table = (Elf32_Phdr *)((char *)map_start + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        func(&phdr_table[i], arg);
    }

    return 0;
}
const char *phdr_type_to_string(Elf32_Word type)
{
    switch (type) {
        case PT_NULL:   return "NULL";
        case PT_LOAD:   return "LOAD";
        case PT_DYNAMIC:return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE:   return "NOTE";
        case PT_PHDR:   return "PHDR";
        default:        return "UNKNOWN";
    }
}
void print_flags(Elf32_Word flags)
{
    if (flags & PF_R) printf("R");
    if (flags & PF_W) printf("W");
    if (flags & PF_X) printf("E");
}
void print_mmap_prot(Elf32_Phdr *phdr)
{
    int printed = 0;

    if (phdr->p_flags & PF_R) {
        printf("PROT_READ");
        printed = 1;
    }
    if (phdr->p_flags & PF_W) {
        printf("%sPROT_WRITE", printed ? " " : "");
        printed = 1;
    }
    if (phdr->p_flags & PF_X) {
        printf("%sPROT_EXEC", printed ? " " : "");
    }
}
void print_phdr(Elf32_Phdr *phdr, int i)
{
    printf("%-7s ", phdr_type_to_string(phdr->p_type));
    printf("0x%06x  ", phdr->p_offset);
    printf("0x%08x  ", phdr->p_vaddr);
    printf("0x%08x  ", phdr->p_paddr);
    printf("0x%05x  ", phdr->p_filesz);
    printf("0x%05x  ", phdr->p_memsz);
    print_flags(phdr->p_flags);
    printf("  ");
    printf("0x%x\n", phdr->p_align);
    printf("  mmap prot: ");
    print_mmap_prot(phdr);
    printf("\n");
    printf("  mmap flags: ");
    printf("MAP_PRIVATE MAP_FIXED");
    printf("\n\n");
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    if (phdr->p_type != PT_LOAD) {
        return;
    }
    uint32_t vaddr = phdr->p_vaddr & 0xfffff000;
    uint32_t offset = phdr->p_offset & 0xfffff000;
    uint32_t padding = phdr->p_vaddr & 0xfff;
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;
    void *map_addr = mmap((void *)vaddr, 
                          phdr->p_memsz + padding, 
                          prot, 
                          MAP_PRIVATE | MAP_FIXED, 
                          fd, 
                          offset);

    if (map_addr == MAP_FAILED) {
        perror("mmap (load_phdr) failed");
        exit(1); 
    }
    // 4. Print information (Reusing your Task 1 logic for reporting)
    printf("Type    Offset    VirtAddr    PhysAddr   FileSiz   MemSiz   Flg  Align\n");
    print_phdr(phdr, 0); 
}
int main(int argc, char** argv){
    FILE* file;
    struct stat st;
    void *map_start;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf-file> {<args>}\n", argv[0]);
        return 1;
    }
    file = fopen(argv[1], "r+");
    if (file < 0) {
        perror("open");
        return 1;
    }
    int fd = fileno(file);
    fstat(fd, &st);
    map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;

    // Task 2b: Load all segments
    foreach_phdr(map_start, (void (*)(Elf32_Phdr *, int))load_phdr, fd);
    /* Task 2d: Transfer control using the startup wrapper */
    /* argc-1: skip loader name. argv+1: start argv from target program name */
    printf("Transferring control to %s at entry point 0x%08x\n", argv[1], ehdr->e_entry);
    startup(argc - 1, argv + 1, (void *)(ehdr->e_entry));
    munmap(map_start, st.st_size);
    fclose(file);
    return 0;
}
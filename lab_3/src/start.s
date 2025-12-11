global _start
global system_call
global main
extern strlen

section .rodata
    newline: db 10          ; newline character
    SYS_OPEN    equ 5
    SYS_WRITE   equ 4
    SYS_READ    equ 3
    O_RDONLY    equ 0
    O_RWC       equ 577     ; Write | Create | Trunc
    PERMS       equ 0644o

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

main:
    push ebp
    mov ebp, esp
    mov ecx, [ebp+8]        ; argc (argument count)
    mov edx, [ebp+12]       ; argv (argument vector pointer)

NextArgv:

    ;argv printing:
    mov eax, [edx]          ; eax = pointer to argv[i] (current string)
    mov ebx, eax            ; ebx = pointer (preserve for write)

    pushad                  ; save registers

    push eax                ; push pointer argument for strlen
    call strlen             ;eax = len(argv[i])
    add esp, 4              ; clean up stack after call

    ;print argv[i]
    ; system_call(SYS_WRITE, STDOUT, buffer, length)
    
    push eax                ; Arg 3: Length (from EAX)
    push ebx                ; Arg 2: Buffer (from EBX)
    push dword [Outfile]            ; Arg 1: FD (STDOUT)
    push SYS_WRITE          ; Syscall Number
    call system_call
    add esp, 16             ; Clean stack

    ; Write a newline to stdout
    push 1                  ; Arg 3: Length 
    push newline            ; Arg 2: Buffer 
    push dword [Outfile]           ; Arg 1: FD (STDOUT)
    push SYS_WRITE          ; Syscall Number
    call system_call
    add esp, 16             ; Clean stack

    ;input/output file handling:
    popad                   ; restore registers saved by pushad, ebx is pointing at argv[i]

    cmp byte [ebx], '-'     ; Check for flag prefix
    jne LoopIncrement       ; If not '-', skip

    cmp byte [ebx+1], 'i'
    je  HandleInput
    
    cmp byte [ebx+1], 'o'
    je  HandleOutput
    
    jmp LoopIncrement       ; Matches '-' but not 'i' or 'o', skip


HandleInput:
    lea ebx, [ebx + 2] ;skip "-i"

    ; system_call(SYS_OPEN, filename, O_RDONLY, 0)
    push 0                  ; Arg 3: Mode (Ignored for read)
    push O_RDONLY           ; Arg 2: Flags
    push ebx                ; Arg 1: Filename
    push SYS_OPEN           ; Syscall Number
    call system_call
    add esp, 16
    
    mov [Infile], eax       ; Update Global Var (Result is in EAX)
    jmp LoopIncrement

HandleOutput:
    ; system_call(SYS_OPEN, filename, O_RWC, PERMS)
    lea ebx, [ebx+2]        ; Skip "-o"
    
    push PERMS              ; Arg 3: Mode (0644)
    push O_RWC              ; Arg 2: Flags
    push ebx                ; Arg 1: Filename
    push SYS_OPEN           ; Syscall Number
    call system_call
    add esp, 16
    
    mov [Outfile], eax      ; Update Global Var
    jmp LoopIncrement

LoopIncrement:
    add edx, 4              ; advance argv pointer to next string
    dec ecx                 ; decrement argc
    jnz NextArgv             ; loop while argc != 0

    jmp Encoder

Encoder:
    sub esp, 4              ; allocate 4 bytes on the stack for input char

read_loop:
    ; system_call(SYS_READ, Infile, buf, 1)
    push 1                  ; Arg 3: Length
    lea eax, [esp+4]        ; Calculate buffer address (current stack + offset)
    push eax                ; Arg 2: Buffer Address
    push dword [Infile]     ; Arg 1: FD
    push SYS_READ           ; Syscall Number
    call system_call
    add esp, 16

    ; Check for End Of File (EOF)
    cmp eax, 0
    jle end_program         ; if 0 or negative, exit loop

    ; If character in range 'A'..'Z', apply Caesar +3 encoding
    cmp byte [esp], 'A'
    jl  print_char          ; if char < 'A', skip encoding
    cmp byte [esp], 'Z'
    jg  print_char          ; if char > 'Z', skip encoding

    ; Uppercase letter: add 3 to the character
    ;todo: wrap around 
    add byte [esp], 3

print_char:
    ; Syscall: write(fd, buf, len)
    push 1                  ; Arg 3
    lea eax, [esp+4]        ; Arg 2: Buffer (Fix offset due to push)
    push eax
    push dword [Outfile]    ; Arg 1
    push SYS_WRITE
    call system_call
    add esp, 16

    ; Continue reading next character
    jmp read_loop

end_program:
    add esp, 4              ; free stack slot
    mov esp, ebp
    pop ebp
    ret                     ; return to caller (start.s will exit)

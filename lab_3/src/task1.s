global main
extern strlen

section .rodata
    newline: db 10          ; newline character

section .data
    Infile  dd 0            ; STDIN file descriptor (0)
    Outfile dd 1            ; STDOUT file descriptor (1)

section .text
main:
    push ebp
    mov ebp, esp
    mov ecx, [ebp+8]        ; argc (argument count)
    mov edx, [ebp+12]       ; argv (argument vector pointer)

Next:
    pushad                  ; save general-purpose registers

    ; Get pointer to current argv string
    mov eax, [edx]          ; eax = pointer to argv[0] (current string)
    mov ebx, eax            ; ebx = pointer (preserve for write)

    ; Call strlen(pointer) to get string length -> eax
    push eax                ; push pointer argument for strlen
    call strlen
    add esp, 4              ; clean up stack after call

    ; Write the argv string to stdout
    mov ecx, ebx            ; ecx = pointer to string
    mov ebx, 1              ; ebx = file descriptor (stdout)
    mov edx, eax            ; edx = length of string (returned by strlen)
    mov eax, 4              ; eax = sys_write
    int 0x80

    ; Write a newline to stdout
    mov ebx, 1              ; ebx = file descriptor (stdout)
    mov ecx, newline        ; ecx = pointer to newline
    mov edx, 1              ; edx = length (1 byte)
    mov eax, 4              ; eax = sys_write
    int 0x80

    popad                   ; restore registers saved by pushad
    add edx, 4              ; advance argv pointer to next string
    dec ecx                 ; decrement argc
    jnz Next                ; loop while argc != 0

    jmp Encoder

; ------------------------
; Encoder: encode stdin -> stdout
; ------------------------
Encoder:
    sub esp, 4              ; allocate 4 bytes on the stack for input char

read_loop:
    ; Syscall: read(fd, buf, len)
    mov eax, 3              ; eax = sys_read
    mov ebx, [Infile]       ; ebx = file descriptor (stdin)
    mov ecx, esp            ; ecx = buffer address (stack slot)
    mov edx, 1              ; edx = number of bytes to read
    int 0x80

    ; Check for End Of File (EOF)
    ; sys_read returns number of bytes read in EAX.
    ; If EAX == 0 -> EOF, if EAX < 0 -> error.
    cmp eax, 0
    jle end_program         ; if 0 or negative, exit loop

    ; If character in range 'A'..'Z', apply Caesar +3 encoding
    cmp byte [esp], 'A'
    jl  print_char          ; if char < 'A', skip encoding
    cmp byte [esp], 'Z'
    jg  print_char          ; if char > 'Z', skip encoding

    ; Uppercase letter: add 3 to the character
    add byte [esp], 3

print_char:
    ; Syscall: write(fd, buf, len)
    mov eax, 4              ; eax = sys_write
    mov ebx, [Outfile]      ; ebx = file descriptor (stdout)
    mov ecx, esp            ; ecx = buffer address
    mov edx, 1              ; edx = length (1 byte)
    int 0x80

    ; Continue reading next character
    jmp read_loop

end_program:
    add esp, 4              ; free stack slot
    mov esp, ebp
    pop ebp
    ret                     ; return to caller (start.s will exit)

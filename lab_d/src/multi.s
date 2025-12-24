global main
extern printf
extern fgets
extern stdin
extern malloc
extern strlen


section .rodata
    newline:    db 10, 0            ; Newline
    string_fmt: db "%s", 10, 0  ; "%s\n"
    int_fmt: db "%d", 10, 0  ; "%d\n"
    hex_fmt:    db "%02hhx", 0     
    
section .bss
    input_buf: resb 600              ; buffer for input

section .data
    x_struct: dd 5                  ; length in bytes
    x_num:    db 0xaa, 1, 2, 0x44, 0x4f ; array of bytes

section .text
print_multi: ; Function: print_multi(struct multi *p)
    push    ebp ;set up stack frame
    mov     ebp, esp
    
    push    esi
    push    ebx

    mov     eax, [ebp+8]       ; eax = pointer to struct p

    mov     ebx, [eax]         ; EBX = p->length
    lea     esi, [eax+4]       ; ESI = p->Array Start

    ; for little-endian printing we go to the end of the array
    add     esi, ebx           ; ESI = start + size
    dec     esi                ; ESI points to last byte

PrintLoop:
    xor     eax, eax           ; clear eax and al
    mov     al, [esi]          ; Load single byte from array

    ; Call printf
    push    eax                ; Value to print
    push    hex_fmt            
    call    printf
    add     esp, 8             ; Clean stack

    ; Update Loop
    dec     esi                ; Move pointer back
    dec     ebx                ; Decrement length
    jnz     PrintLoop        ; If length != 0, keep going

    ; Print Newline
    push    newline
    call    printf
    add     esp, 4

    ; Restore registers and return
    pop     ebx
    pop     esi
    mov     esp, ebp
    pop     ebp
    ret

; Helper: ascii_to_hex reference: //todo make readable, add reference
; Input: AL (ASCII char)
; Output: AL (Hex value 0-15)
ascii_to_hex:
    cmp     al, '9'
    jbe     is_digit
    and     al, 0xDF            ; Convert 'a' (0x61) to 'A' (0x41)
    sub     al, 55              ; 'A' (65) - 55 = 10
    ret
is_digit:
    sub     al, '0'             ; '0' (48) - 48 = 0
    ret

get_multi:
    push    ebp
    mov     ebp, esp
    
    push    ebx ;save callee-saved registers
    push    esi
    push    edi

    ; Call fgets(input_buf, 600, stdin)
    push    dword [stdin]      ; Arg 3: File stream (stdin)
    push    600                ; Arg 2: Max characters to read [cite: 42]
    push    input_buf          ; Arg 1: Buffer to store data
    call    fgets
    add     esp, 12            ; Clean stack (3 args * 4 bytes)
    
    ;Calculate input length using strlen
    push    input_buf
    call    strlen ; eax = strlen(input_buf)
    add     esp, 4

    cmp     eax, 0  ; check for empty string
    je      empty_input        
    
    dec     eax                 ; Decrease length count to exclude newline

calc_length:
    mov     ecx, eax            ; ecx = digit Count in input
    add     eax, 1
    shr     eax, 1              ; eax = number of bytes in array = (digit count + 1) / 2

    push    ecx             
    push    eax            
    
    lea     ebx, [eax + 4]  ; Total size = size_field + array
    push    ebx
    call    malloc ; eax = pointer to allocated struct
    add     esp, 4
    
    pop     edx             ; Restore byte size into EDX
    pop     ecx             ; Restore digit count into ECX

    mov     [eax], edx         ; Store length into struct

    lea     esi, [input_buf + ecx - 1] ; Point to last digit in input
    lea     edi, [eax + 4]             ; Point to start of array in struct
    mov     ebx, eax        ; Save struct pointer in ebx

parse_loop:
    cmp     esi, input_buf      ; Check if we reached the start of input
    jl      done_parsing

    ;Low Nibble (Last Char)
    xor     eax, eax
    mov     al, [esi]           ; Load char 
    call    ascii_to_hex        ; Convert to hex
    dec     esi                 ; Move to next char

    ; High Nibble (Second to Last Char)
    cmp     esi, input_buf      ; Check if we ran out of chars (Odd length case)
    jl      store_byte         ; If odd, just store the low nibble

    xor     edx, edx
    mov     dl, [esi]           ; Load char 
    push    eax                 ; Save Low Nibble

    mov     al, dl
    call    ascii_to_hex        ; Convert High Nibble to hex
    shl     al, 4               ; Shift High Nibble into high 4 bits of AL to make space for Low Nibble
    pop     edx                 ; Restore Low Nibble
    or      al, dl              ; Combine low and high nibble into byte
    dec     esi                 ; Move to next char

store_byte:
    mov     [edi], al           ; Store byte in struct array
    inc     edi                 ; Move to next byte in array
    jmp     parse_loop

done_parsing:
    mov     eax, ebx            ; Restore struct pointer to EAX for return
    jmp     end_get_multi

empty_input:
    xor     eax, eax

end_get_multi:
    pop     edi ;restore callee-saved registers
    pop     esi
    pop     ebx

    mov     esp, ebp
    pop     ebp
    ret

 ; Function: get_max_min
; Input: eax = ptr to struct A, ebx = ptr to struct B
; Output: eax = ptr to longer struct, ebx = ptr to shorter struct
get_max_min:
    push    ecx
    push    edx

    mov     ecx, [eax]      ; Length of A
    mov     edx, [ebx]      ; Length of B
    
    cmp     ecx, edx        ; Compare Len(A) vs Len(B)
    ja      no_swap        ; If A > B, we are good (eax is max)

    ; If A <= B, we need to swap EAX and EBX
    xchg    eax, ebx        ; Swap registers

no_swap:
    pop     edx
    pop     ecx
    ret

; arg1: struct A ptr, arg2: struct B ptr
; Returns pointer to new struct C = A + B
add_multi: 
    push    ebp
    mov     ebp, esp
    push    ebx             ; Save callee-saved registers
    push    esi
    push    edi

    ; Load Arguments and Sort
    mov     eax, [ebp+8]    ; Arg1
    mov     ebx, [ebp+12]   ; Arg2
    call    get_max_min     ; eax = Max, ebx = Min

    mov     ecx, [eax]      ; Max Len = struct A length
    add     ecx, 1          ; Len of C = Len(A) + 1 (for carry)

    push    eax             ; Save Max Ptr
    push    ebx             ; Save Min Ptr
    push    ecx             ; Save Result Len

    ;allocate struct C
    lea     edx, [ecx+4]    ; Total bytes = length field + array
    push    edx
    call    malloc          ; eax = ptr to struct C
    add     esp, 4

    pop     ecx             ; ecx = length of C
    pop     ebx             ; ebx = ptr to Min struct
    pop     edx             ; edx = ptr to Max struct

    mov     [eax], ecx      ; Store length in new struct

    ; // todo: continue implementing addition logic here
    
main:
    push ebp
    mov ebp, esp           
    mov ecx, [ebp+8]       ; Get first argument ac
    mov edx, [ebp+12]      ; Get 2nd argument av

    pushad              ; Save argc
    push dword ecx       ; push av[i] (i=0 first)
    push dword int_fmt
    call printf            ; printf(string_fmt, [edx])
    add esp, 8             ; "remove" printf arguments
    popad

NextArgv:
    pushad
    push dword [edx]       ; push av[i] (i=0 first)
    push dword string_fmt
    call printf            ; printf(string_fmt, [edx])
    add esp, 8             ; "remove" printf arguments
    popad

    add edx, 4             ; advance edx to &av[i+1]
    dec ecx                ; dec. arg counter
    jnz NextArgv               ; loop if not yet zero

Task1:
    ; ;task 1a
    ; push x_struct          ; Push address of the struct
    ; call print_multi
    ; add esp, 4             ; Clean stack

    ; --- Task 1.B 
    call    get_multi          ; Read user input into input_buf

    push    eax                 ; passing struct pointer
    call    print_multi
    add     esp, 4

EndMain:
    mov esp, ebp
    pop ebp
    ret
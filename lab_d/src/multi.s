global main
extern printf
extern fgets
extern stdin
extern malloc
extern strlen

section .rodata
    newline:    db 10, 0                ; Newline
    string_fmt: db "%s", 10, 0          ; "%s\n"
    int_fmt:    db "%d", 10, 0          ; "%d\n"
    hex_fmt:    db "%02hhx", 0          ; Hexadecimal format

section .bss
    input_buf:  resb 600                ; Buffer for input

section .data
    x_struct:   dd 5                    ; Length in bytes
    x_num:      db 0xaa, 1, 2, 0x44, 0x4f ; Array of bytes
    y_struct:   dd 6                    ; Length in bytes
    y_num:      db 0xaa, 1, 2, 3, 0x44, 0x4f ; Array of bytes

    lfsr_state: dw 0xB0B0               ; Initial Seed 
    mask:       dw 0xB400               ; 16-bit Fibonacci LFSR taps (1, 11, 13, 14, 16)

section .text
print_multi:                            ; Function: print_multi(struct multi *p)
    push    ebp                         ; Set up stack frame
    mov     ebp, esp
    
    push    esi
    push    ebx

    mov     eax, [ebp+8]                ; eax = pointer to struct p

    mov     ebx, [eax]                  ; EBX = p->length
    lea     esi, [eax+4]                ; ESI = p->Array Start

    ; For little-endian printing we go to the end of the array
    add     esi, ebx                    ; ESI = start + size
    dec     esi                         ; ESI points to last byte

PrintLoop:
    xor     eax, eax                    ; Clear eax and al
    mov     al, [esi]                   ; Load single byte from array

    ; Call printf
    push    eax                         ; Value to print
    push    hex_fmt            
    call    printf
    add     esp, 8                      ; Clean stack

    ; Update Loop
    dec     esi                         ; Move pointer back
    dec     ebx                         ; Decrement length
    jnz     PrintLoop                   ; If length != 0, keep going

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

ascii_to_hex:                           ; Helper: ascii_to_hex
                                        ; Input: AL (ASCII char)
                                        ; Output: AL (Hex value 0-15)
    cmp     al, '9'
    jbe     is_digit
    and     al, 0xDF                    ; Convert 'a' (0x61) to 'A' (0x41)
    sub     al, 55                      ; 'A' (65) - 55 = 10
    ret
is_digit:
    sub     al, '0'                     ; '0' (48) - 48 = 0
    ret

get_multi:
    push    ebp
    mov     ebp, esp
    
    push    ebx                         ; Save callee-saved registers
    push    esi
    push    edi

    ; Call fgets(input_buf, 600, stdin)
    push    dword [stdin]               ; Arg 3: File stream (stdin)
    push    600                         ; Arg 2: Max characters to read
    push    input_buf                   ; Arg 1: Buffer to store data
    call    fgets
    add     esp, 12                     ; Clean stack (3 args * 4 bytes)
    
    ; Calculate input length using strlen
    push    input_buf
    call    strlen                      ; eax = strlen(input_buf)
    add     esp, 4

    cmp     eax, 0                      ; Check for empty string
    je      empty_input        
    
    dec     eax                         ; Decrease length count to exclude newline

calc_length:
    mov     ecx, eax                    ; ecx = digit count in input
    add     eax, 1
    shr     eax, 1                      ; eax = number of bytes in array = (digit count + 1) / 2

    push    ecx             
    push    eax            
    
    lea     ebx, [eax + 4]              ; Total size = size_field + array
    push    ebx
    call    malloc                      ; eax = pointer to allocated struct
    add     esp, 4
    
    pop     edx                         ; Restore byte size into EDX
    pop     ecx                         ; Restore digit count into ECX

    mov     [eax], edx                  ; Store length into struct

    lea     esi, [input_buf + ecx - 1]  ; Point to last digit in input
    lea     edi, [eax + 4]              ; Point to start of array in struct
    mov     ebx, eax                    ; Save struct pointer in ebx

parse_loop:
    cmp     esi, input_buf              ; Check if we reached the start of input
    jl      done_parsing

    ; Low Nibble (Last Char)
    xor     eax, eax
    mov     al, [esi]                   ; Load char 
    call    ascii_to_hex                ; Convert to hex
    dec     esi                         ; Move to next char

    ; High Nibble (Second to Last Char)
    cmp     esi, input_buf              ; Check if we ran out of chars (Odd length case)
    jl      store_byte                  ; If odd, just store the low nibble

    xor     edx, edx
    mov     dl, [esi]                   ; Load char 
    push    eax                         ; Save Low Nibble

    mov     al, dl
    call    ascii_to_hex                ; Convert High Nibble to hex
    shl     al, 4                       ; Shift High Nibble into high 4 bits of AL to make space for Low Nibble
    pop     edx                         ; Restore Low Nibble
    or      al, dl                      ; Combine low and high nibble into byte
    dec     esi                         ; Move to next char

store_byte:
    mov     [edi], al                   ; Store byte in struct array
    inc     edi                         ; Move to next byte in array
    jmp     parse_loop

done_parsing:
    mov     eax, ebx                    ; Restore struct pointer to EAX for return
    jmp     end_get_multi

empty_input:
    xor     eax, eax

end_get_multi:
    pop     edi                         ; Restore callee-saved registers
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

    mov     ecx, [eax]                  ; Length of A
    mov     edx, [ebx]                  ; Length of B
    
    cmp     ecx, edx                    ; Compare Len(A) vs Len(B)
    ja      no_swap                     ; If A > B, we are good (eax is max)

    ; If A <= B, we need to swap EAX and EBX
    xchg    eax, ebx                    ; Swap registers

no_swap:
    pop     edx
    pop     ecx
    ret

; arg1: struct A ptr, arg2: struct B ptr
; Returns pointer to new struct C = A + B
add_multi: 
    push    ebp
    mov     ebp, esp
    push    ebx                         ; Save callee-saved registers
    push    esi
    push    edi

    ; Load Arguments and Sort
    mov     eax, [ebp+8]                ; Arg1
    mov     ebx, [ebp+12]               ; Arg2
    call    get_max_min                 ; eax = Max, ebx = Min

    mov     ecx, [eax]                  ; Max Len = struct A length
    add     ecx, 1                      ; Len of C = Len(A) + 1 (for carry)

    push    eax                         ; Save Max Ptr
    push    ebx                         ; Save Min Ptr
    push    ecx                         ; Save Result Len

    ; Allocate struct C
    lea     edx, [ecx+4]                ; Total bytes = length field + array
    push    edx
    call    malloc                      ; eax = ptr to struct C
    add     esp, 4

    pop     ecx                         ; ecx = length of C
    pop     ebx                         ; ebx = ptr to Min struct
    pop     edx                         ; edx = ptr to Max struct

    mov     [eax], ecx                  ; Store length in new struct
    push    eax                         ; Save pointer to struct C

    ; Setup pointers for addition loop
    mov     ecx, [ebx]                  ; ecx = length of Min struct
    lea     esi, [ebx+4]                ; esi = start of Min array
    lea     ebx, [edx+4]                ; ebx = start of Max array
    lea     edi, [eax+4]                ; edi = start of C array

    clc                                 ; Clear carry flag

shared_loop:
    mov     al, [esi]                   ; Load byte from Min array
    adc     al, [ebx]                   ; Add byte from Max array + Carry Flag
    mov     [edi], al                   ; Store result in C

    inc     esi                         ; Move all pointers forward
    inc     ebx             
    inc     edi
    dec     ecx                         ; Decrement Min length
    jnz     shared_loop                 ; Loop if Min length not zero
    pushf                               ; Save flags (carry)

    ; Handle remaining bytes in Max array
    mov     ecx, [edx]                  ; Load Max Length
    add     ecx, edx                    ; ecx = address of Max Struct + Max Struct Length
    add     ecx, 4                      ; Adjust for length field (4 bytes) -> ECX is now the address of the final byte of Max array
    sub     ecx, ebx                    ; ECX = End Address - Current Pointer (EBX) = Remaining Bytes to process

    jz      skip_remaining                 ; If no remaining bytes, check for final carry

    popf                                ; Restore flags (carry) for remaining loop,
                                        ; since calculating remaining bytes cleared it

remaining_loop:
    mov     al, 0                       ; Prepare 0
    adc     al, [ebx]                   ; Add 0 + Max Byte + Carry 
    mov     [edi], al                   ; Store result in C

    inc     ebx                         ; Move pointers
    inc     edi
    dec     ecx                         ; Decrement counter
    jnz     remaining_loop
    jmp     final_carry

skip_remaining:
    popf                                ; Restore flags for final carry check

final_carry:
    mov     al, 0
    adc     al, 0                       ; Add any lingering carry to 0
    mov     [edi], al                   ; Store in the extra byte we allocated for C    

    pop     eax                         ; Restore pointer to struct C for return
    pop     edi                         ; Restore callee-saved registers
    pop     esi
    pop     ebx
    mov     esp, ebp                    ; Restore stack frame
    pop     ebp
    ret

rand_num:                               ; Output: al = random 8-bit number
    push    ebp                         ; Stack frame
    mov     ebp, esp
    push    ebx                         ; Save callee-saved registers

    mov     ecx, 8                      ; ecx is bit counter
    mov     bx, [lfsr_state]            ; Load current LFSR state
    xor     ax, ax                      ; Clear ax to store result

lfsr_loop:
    mov     ax, bx
    and     ax, [mask]                  ; Apply mask to get tap bits

    ; Calculate parity of masked ax, JP only checks lower 8 bits, so we need to fold upper bits into lower
    mov     dx, ax 
    shr     dx, 8                       ; Shift upper byte into lower
    xor     ax, dx                      ; XOR upper and lower bytes, now parity is in lower 8 bits

    jp      even_parity                 ; JP jumps if Parity Flag (PF) is 1 (Even)
    
    ; If we are here, Parity is Odd (XOR sum is 1)
    stc                                 ; Set Carry Flag = 1
    jmp     shift_state

even_parity:
    ; If we are here, Parity is Even (XOR sum is 0)
    clc                                 ; Carry Flag = 0

shift_state:
    rcr     bx, 1                       ; Rotate Right with Carry
                                        ; Shifts all bits right, LSB gets removed from state, MSB is assigned carry Flag

    dec     ecx
    jnz     lfsr_loop                   ; Loop until 8 bits generated

    mov     [lfsr_state], bx            ; Store updated LFSR state
    mov     al, bl                      ; Return lower 8 bits as random number
    and     eax, 0xFF                   ; Clear upper bits, keep AL

    pop     ebx                         ; Restore callee-saved registers
    mov     esp, ebp
    pop     ebp
    ret

PRmulti:                                ; Output: eax = pointer to pseudo-random multi-precision integer struct
    push    ebp                         ; Stack frame
    mov     ebp, esp
    push    ebx                         ; Save callee-saved registers
    push    esi
    push    edi

get_len_loop:                           ; Get non-zero length for integer
    call    rand_num                    ; Get random byte
    cmp     al, 0
    jz      get_len_loop                ; Retry if 0

    movzx   esi, al                     ; esi = length

    ; Allocate Struct
    lea     eax, [esi + 4]              ; Size + Header
    push    eax
    call    malloc
    add     esp, 4

    mov     [eax], esi                  ; Store length in struct
    mov     ebx, eax                    ; ebx = Struct Pointer
    lea     edi, [eax + 4]              ; edi = Pointer to Struct Array Start

fill_loop:
    call    rand_num                    ; al = random byte
    mov     [edi], al                   ; Store Random Byte in Struct
    
    inc     edi                         ; Move to next byte
    dec     esi                         ; Decrement length
    jnz     fill_loop

    mov     eax, ebx                    ; Restore Pointer
    
    pop     edi                         ; Restore callee-saved registers
    pop     esi
    pop     ebx
    mov     esp, ebp                    ; Restore stack frame
    pop     ebp
    ret

main:
    push    ebp
    mov     ebp, esp           
    mov     ecx, [ebp+8]                ; Get first argument ac
    mov     edx, [ebp+12]               ; Get 2nd argument av

    cmp     ecx, 1                      ; If no args, go to default mode
    jle     default_mode

    mov     edx, [edx+4]                ; Get first argument string av[1]
    mov     ax, [edx]
    cmp     ax, 0x492D                  ; argv[1] == "-I"?
    je      input_mode

    cmp     ax, 0x522D                  ; argv[1] == "-R"?
    je      random_mode

default_mode:
    push    x_struct                    ; Push X and Y
    call    print_multi                 ; Print X
    add     esp, 4
    
    push    y_struct
    call    print_multi                 ; Print Y
    add     esp, 4
    
    push    x_struct                    ; Push X and Y for addition
    push    y_struct
    call    add_multi                   ; Add X + Y
    add     esp, 8
    push    eax
    call    print_multi                 ; Print result
    add     esp, 4
    jmp     EndMain

input_mode:
    call    get_multi                   ; Read integer A from stdin
    push    eax                         ; Push for addition
    
    push    eax                         ; Push copy for printing
    call    print_multi        
    add     esp, 4             

    call    get_multi                   ; Read integer B from stdin
    push    eax                         ; Push for addition
    
    push    eax                         ; Push copy for printing
    call    print_multi        
    add     esp, 4     
    
    call    add_multi                   ; Add A + B (args already on stack)
    add     esp, 8

    push    eax
    call    print_multi                 ; Print result
    add     esp, 4
    jmp     EndMain

random_mode:
    call    PRmulti                     ; Get random integer A
    push    eax                         ; Push for addition
    
    push    eax                         ; Push copy for printing
    call    print_multi        
    add     esp, 4  

    call    PRmulti                     ; Get random integer B
    push    eax                         ; Push for addition
    
    push    eax                         ; Push copy for printing
    call    print_multi        
    add     esp, 4  
    
    call    add_multi                   ; Add A + B
    add     esp, 8

    push    eax
    call    print_multi                 ; Print result
    add     esp, 4
    jmp     EndMain

EndMain:
    mov     esp, ebp
    pop     ebp
    ret
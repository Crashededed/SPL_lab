global _start

section .rodata
 Str1: db "Hello World", 10, 0
 DummyStr:
section .data

 outfile: dd 1 ; int outfile=1
section .text

_start: mov edx, DummyStr-Str1-1 ; Byte count
 mov ecx, Str1
 mov ebx, [outfile]
 mov eax, 4
 int 0x80 ; Linux system call
 mov ebx, 0
 mov eax, 1
 int 0x80 ; Linux system call
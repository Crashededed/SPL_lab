# All Targets
all: task1

task1: util.o start.o
	ld -m elf_i386  util.o start.o -o task1
	rm -f util.o start.o
	
# Depends on the source files
util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o: start.s
	nasm -f elf32 start.s -o start.o

# Tell make that "clean" is not a file name!
.PHONY: clean 

# Clean the build directory
clean:
	rm -rf *
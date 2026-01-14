# All Targets
all: link

link: multi.o
	gcc -m32 multi.o -o multi
	rm -f multi.o

multi.o: 
	nasm -f elf32 multi.s -o multi.o
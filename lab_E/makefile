all: clean myELF

myELF: myELF.o
	gcc -m32 -g -Wall -o myELF myELF.o
	rm -f myELF.o

myELF.o: myELF.c
	gcc -g -Wall -m32 -D_FILE_OFFSET_BITS=64 -c -o myELF.o myELF.c

.PHONY: clean all

# Clean the build directory
clean:
	rm -f myELF myELF.o
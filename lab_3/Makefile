# All Targets
all: bin clean task0 task1 hello

# Create bin directory
bin:
	mkdir -p bin

# Executable "sum_program" depends on the object files
task0: bin/main.o bin/util.o bin/start.o
	ld -m elf_i386 bin/start.o bin/main.o bin/util.o -o bin/task0
	rm -f bin/main.o bin/start.o bin/util.o

task1: bin/task1.o bin/util.o bin/start.o
	ld -m elf_i386 bin/task1.o bin/util.o bin/start.o -o bin/task1
	rm -f bin/task1.o bin/util.o bin/start.o

hello:
	nasm -f elf32 src/hello.s -o bin/hello.o
	ld -m elf_i386 bin/hello.o -o bin/hello
	rm -f bin/hello.o
	
# Depends on the source files
bin/util.o: src/util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector src/util.c -o bin/util.o

bin/main.o: src/main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector src/main.c -o bin/main.o

bin/start.o: src/start.s
	nasm -f elf32 src/start.s -o bin/start.o

bin/task1.o: src/task1.s
	nasm -f elf32 src/task1.s -o bin/task1.o
	
# Tell make that "clean" is not a file name!
.PHONY: clean 

# Clean the build directory
clean:
	rm -rf bin/*
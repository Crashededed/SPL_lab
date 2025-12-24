# All Targets
all: bin clean link

# Create bin directory
bin:
	mkdir -p bin

link: bin/multi.o
	gcc -m32 bin/multi.o -o bin/multi
	rm -f bin/multi.o

bin/multi.o: 
	nasm -f elf32 src/multi.s -o bin/multi.o
	
# Tell make that "clean" is not a file name!
.PHONY: clean 

# Clean the build directory
clean:
	rm -rf bin/*
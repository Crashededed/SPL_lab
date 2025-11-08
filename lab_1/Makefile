# All Targets
all: bin clean link 

# Create bin directory
bin:
	mkdir -p bin

# Executable "sum_program" depends on the object files
link: bin/count-words.o
	gcc -m32 -g -Wall -o bin/count-words bin/count-words.o 
	rm -f bin/count-words.o 

# Depends on the source files
bin/count-words.o: src/count-words.c
	gcc -g -Wall -m32 -c -o bin/count-words.o src/count-words.c

# Clean the build directory
clean:
	rm -f bin/*
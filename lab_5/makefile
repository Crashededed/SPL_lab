all: bin clean my_loader 

# Create bin directory
bin:
	mkdir -p bin

my_loader: bin/my_loader.o
	gcc -m32 -g -Wall -o bin/my_loader bin/my_loader.o 
	rm -f bin/my_loader.o

bin/my_loader.o: src/my_loader.c
	gcc -g -Wall -m32 -c -o bin/my_loader.o src/my_loader.c

.PHONY: clean all

# Clean the build directory
clean:
	rm -f bin/*
# All Targets
all: bin clean myshell

# Create bin directory
bin:
	mkdir -p bin

# Executable "myshell" depends on the object files
myshell: bin/myshell.o bin/LineParser.o
	gcc -m32 -g -Wall -o bin/myshell bin/myshell.o bin/LineParser.o 
	rm -f bin/myshell.o bin/LineParser.o 

# Depends on the source files
bin/myshell.o: src/myshell.c src/LineParser.h
	gcc -g -Wall -m32 -c -o bin/myshell.o src/myshell.c 
	
bin/LineParser.o: src/LineParser.c src/LineParser.h
	gcc -g -Wall -m32 -c -o bin/LineParser.o src/LineParser.c 

# Tell make that "clean" is not a file name!
.PHONY: clean all

# Clean the build directory
clean:
	rm -f bin/*
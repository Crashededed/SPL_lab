# All Targets
all: bin clean link 

# Create bin directory
bin:
	mkdir -p bin

# Executable "count-words" depends on the object files
link: bin/count-words.o bin/addresses.o bin/base.o bin/menu_map.o
	gcc -m32 -g -Wall -o bin/count-words bin/count-words.o 
	gcc -m32 -g -Wall -o bin/addresses bin/addresses.o
	gcc -m32 -g -Wall -o bin/base bin/base.o
	gcc -m32 -g -Wall -o bin/menu_map bin/menu_map.o
	rm -f bin/count-words.o bin/addresses.o bin/base.o bin/menu_map.o

# Depends on the source files
bin/count-words.o: src/count-words.c
	gcc -g -Wall -m32 -c -o bin/count-words.o src/count-words.c

bin/addresses.o: src/addresses.c
	gcc -g -Wall -m32 -c -o bin/addresses.o src/addresses.c

bin/base.o: src/base.c
	gcc -g -Wall -m32 -c -o bin/base.o src/base.c

bin/menu_map.o: src/menu_map.c
	gcc -g -Wall -m32 -c -o bin/menu_map.o src/menu_map.c

# Clean the build directory
clean:
	rm -f bin/*
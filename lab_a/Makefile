# All Targets
all: bin encoder

# Create bin directory
bin:
	mkdir -p bin

# Tool invocations

# Executable "sum_program" depends on the object files
sum_program: bin/main.o bin/numbers.o bin/add.o
	gcc -m32 -g -Wall -o bin/sum_program bin/main.o bin/numbers.o bin/add.o
	rm -f bin/main.o bin/numbers.o bin/add.o

# Executable "my_echo" depends on my_echo.o
my_echo: bin/my_echo.o
	gcc -m32 -g -Wall -o bin/my_echo bin/my_echo.o
	rm -f bin/my_echo.o

# Executable "encoder" depends on encoder.o
encoder: bin/encoder.o
	gcc -m32 -g -Wall -o bin/encoder bin/encoder.o
	rm -f bin/encoder.o

# Depends on the source files
bin/main.o: src/main.c
	gcc -g -Wall -m32 -c -o bin/main.o src/main.c

bin/numbers.o: src/numbers.c
	gcc -g -Wall -m32 -c -o bin/numbers.o src/numbers.c

bin/my_echo.o: src/my_echo.c
	gcc -g -Wall -m32 -c -o bin/my_echo.o src/my_echo.c

bin/encoder.o: src/encoder.c
	gcc -g -Wall -m32 -c -o bin/encoder.o src/encoder.c

bin/add.o: src/add.s
	nasm -g -f elf -w+all -o bin/add.o src/add.s

# Tell make that "clean" is not a file name!
.PHONY: clean 

# Clean the build directory
clean:
	rm -rf bin
all: bin clean task1 

# Create bin directory
bin:
	mkdir -p bin

task1: task1.o task4.o
	gcc -m32 -g -Wall -o task1 task1.o 
	rm -f task1.o

task1.o: task1.c
	gcc -g -Wall -m32 -c -o task1.o task1.c

task4.o: task4.c
	gcc -m32 -fno-pie -fno-stack-protector -c -o task4.o task4.c

.PHONY: clean all

# Clean the build directory
clean:
	rm -f *
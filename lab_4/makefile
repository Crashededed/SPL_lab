all: bin clean hexeditplus 

# Create bin directory
bin:
	mkdir -p bin

hexeditplus: bin/hexeditplus.o bin/task4.o
	gcc -m32 -g -Wall -o bin/hexeditplus bin/hexeditplus.o 
	rm -f bin/hexeditplus.o

bin/hexeditplus.o: src/hexeditplus.c
	gcc -g -Wall -m32 -c -o bin/hexeditplus.o src/hexeditplus.c

bin/task4.o: src/task4.c
	gcc -m32 -fno-pie -fno-stack-protector -c -o bin/task4.o src/task4.c

.PHONY: clean all

# Clean the build directory
clean:
	rm -f bin/*
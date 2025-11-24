# All Targets
all: clean encoder

# Executable "encoder" depends on encoder.o
encoder: encoder.o
	gcc -m32 -g -Wall -o encoder encoder.o
	rm -f encoder.o

encoder.o: encoder.c
	gcc -g -Wall -m32 -c -o encoder.o encoder.c

# Tell make that "clean" is not a file name!
.PHONY: clean 

# Clean the build directory
clean:
	rm -rf *.o encoder
# All Targets
all: AntiVirus

# # Create bin directory
# bin:
# 	mkdir -p bin

AntiVirus: AntiVirus.o
	gcc -m32 -g -Wall -o virusDetector AntiVirus.o 
	rm -f AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -g -Wall -m32 -c -o AntiVirus.o AntiVirus.c 

# Tell make that "clean" is not a file name!
.PHONY: clean all

# Clean the build directory
clean:
	rm -f virusDetector
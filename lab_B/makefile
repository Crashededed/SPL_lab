# All Targets
all: clean AntiVirus

# # Create bin directory
# bin:
# 	mkdir -p bin

# Executable "lab3_bubblesort" depends on the object files
# lab3_bubblesort: bin/lab3_bubblesort.o
# 	gcc -m32 -g -Wall -o bin/lab3_bubblesort bin/lab3_bubblesort.o 
# 	rm -f bin/lab3_bubblesort.o

# hexaPrint: bin/hexaPrint.o
# 	gcc -m32 -g -Wall -o bin/hexaPrint bin/hexaPrint.o 
# 	rm -f bin/hexaPrint.o
AntiVirus: bin/AntiVirus.o
	gcc -m32 -g -Wall -o bin/AntiVirus bin/AntiVirus.o 
	rm -f bin/AntiVirus.o



# Depends on the source files
# bin/lab3_bubblesort.o: src/lab3_bubblesort.c
# 	gcc -g -Wall -m32 -c -o bin/lab3_bubblesort.o src/lab3_bubblesort.c 

# bin/hexaPrint.o: src/hexaPrint.c
# 	gcc -g -Wall -m32 -c -o bin/hexaPrint.o src/hexaPrint.c 

bin/AntiVirus.o: src/AntiVirus.c
	gcc -g -Wall -m32 -c -o bin/AntiVirus.o src/AntiVirus.c 

# Tell make that "clean" is not a file name!
.PHONY: clean all

# Clean the build directory
clean:
	rm -f bin/AntiVirus
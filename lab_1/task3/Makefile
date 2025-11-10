# All Targets
all: clean link 

# Executable "count-words" depends on the object files
link: menu_map.o
	gcc -m32 -g -Wall -o menu_map menu_map.o
	rm -f menu_map.o

menu_map.o: menu_map.c
	gcc -g -Wall -m32 -c -o menu_map.o menu_map.c

# Clean the build directory
clean:
	rm -f menu_map menu_map.o
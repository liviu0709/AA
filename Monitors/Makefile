CFLAGS=-Wall -Werror -g

build: main.c
	gcc main.c graf.c $(CFLAGS) -o monitoare

run: build
	./monitoare

clean:
	rm -f monitoare

valgrind: build
	valgrind -s --show-leak-kinds=all --leak-check=yes --track-origins=yes ./monitoare

CFLAGS=-Wall -Werror -g

build: main.c
	gcc main.c turing.c $(CFLAGS) -o main

run: build
	./main

clean:
	rm -f main debug

debug:
	gcc main.c turing.c $(CFLAGS) -o debug

valgrind: build
	valgrind -s --show-leak-kinds=all --leak-check=yes --track-origins=yes ./main

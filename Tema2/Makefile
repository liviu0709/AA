CFLAGS=-Wall -Werror -g
LOLOLI=-fsanitize=address

build: main.c
	gcc main.c $(CFLAGS) -o tema2

run: build
	./tema2 $(INPUT) $(OUTPUT)

clean:
	rm -f tema2

pack:
	zip solver.zip main.c Makefile

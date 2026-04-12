CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lncurses


main: main.c
	$(CC) main.c -o main $(CFLAGS) $(LDFLAGS)

run: main
	./main

clean:
	rm main

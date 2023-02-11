CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

.PHONY: all

all: httpserver

httpserver: httpserver.o bind.o
	$(CC) -o httpserver bind.o httpserver.o

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c

bind.o: bind.c
	$(CC) $(CFLAGS) -c bind.c

clean:
	rm -f httpserver httpserver.o bind.o

format:
	clang-format -i -style=file *.[ch]

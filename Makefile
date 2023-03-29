CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

.PHONY: all

all: httpserver

httpserver: httpserver.o 
	$(CC) -o httpserver httpserver.o

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c

clean:
	rm -f httpserver httpserver.o 

format:
	clang-format -i -style=file *.[ch]

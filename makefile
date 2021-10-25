all:server
CFLAGS = -g -Wall -Werror -Wno-format-truncation -pthread -O0
cc = gcc
server : httpserver.o
	$(CC) $(CFLAGS) -o server httpserver.o
	
httpserver.o : httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c
	
clean:
	rm -f server*.o

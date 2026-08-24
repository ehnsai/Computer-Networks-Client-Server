CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = server client parserver

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

parserver: parserver.c
	$(CC) $(CFLAGS) -o parserver parserver.c

clean:
	rm -f $(TARGETS)

rebuild: clean all

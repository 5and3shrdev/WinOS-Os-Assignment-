CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: os_model

os_model: main.o scheduler.o memory.o filesystem.o sync.o
	$(CC) $(CFLAGS) -o os_model main.o scheduler.o memory.o filesystem.o sync.o

main.o: main.c scheduler.h memory.h filesystem.h sync.h
	$(CC) $(CFLAGS) -c main.c

scheduler.o: scheduler.c scheduler.h
	$(CC) $(CFLAGS) -c scheduler.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

filesystem.o: filesystem.c filesystem.h
	$(CC) $(CFLAGS) -c filesystem.c

sync.o: sync.c sync.h
	$(CC) $(CFLAGS) -c sync.c

clean:
	rm -f *.o os_model

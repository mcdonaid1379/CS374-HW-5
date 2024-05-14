CC = gcc
CFLAGS = -g
exe_file = smallsh

$(exe_file): main.o
	$(CC) $(CFLAGS) main.o -o $(exe_file)

main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o $(exe_file)

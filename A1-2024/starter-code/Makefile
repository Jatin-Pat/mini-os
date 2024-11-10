CC=gcc
CFLAGS= -g -Wall -pthread

mysh: shell.c interpreter.c shellmemory.c schedulermemory.c errors.c setup.c codememory.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c schedulermemory.c errors.c setup.c codememory.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o schedulermemory.o errors.o setup.o codememory.c

clean: 
	rm mysh; rm *.o

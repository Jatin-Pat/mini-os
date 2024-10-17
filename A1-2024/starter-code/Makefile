CC=gcc
CFLAGS= -g -Wall

mysh: shell.c interpreter.c shellmemory.c scheduler_memory.c errors.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c scheduler_memory.c errors.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o scheduler_memory.o errors.o

clean: 
	rm mysh; rm *.o

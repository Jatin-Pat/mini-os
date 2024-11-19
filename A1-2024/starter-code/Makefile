CC=gcc
CFLAGS= -g -Wall -pthread

mysh: shell.c interpreter.c shellmemory.c schedulermemory.c errors.c setup.c setup.h codememory.c scheduler.c
	$(CC) $(CFLAGS) -D CODE_MEM_SIZE=$(framesize) -D VAR_MEM_SIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c schedulermemory.c errors.c resourcemanager.c setup.c codememory.c scheduler.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o schedulermemory.o errors.o resourcemanager.o setup.o codememory.o scheduler.o

clean: 
	rm mysh; rm *.o

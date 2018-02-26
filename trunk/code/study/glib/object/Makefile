CC = gcc
CC_FLAGS = `pkg-config --cflags glib-2.0 gobject-2.0`
all:
	$(CC) -c boy.c $(CC_FLAGS)
	$(CC) -c man.c $(CC_FLAGS)
	$(CC) -c main.c $(CC_FLAGS)
	$(CC) -o simple boy.o man.o main.o `pkg-config --libs glib-2.0 gobject-2.0`


CC ?= gcc CFLAGS = -Wall -g LDFLAGS = -Wall -g

all: writer

writer: writer.o $(CC) $(LDFLAGS) writer.o -o writer

writer.o: writer.c $(CC) $(CFLAGS) -c writer.c

clean: rm -f writer *.o

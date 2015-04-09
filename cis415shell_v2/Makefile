CFLAGS=-g -Wall
CC=gcc
SRCS=tokenizer.c halvolnyShell.c utils.c
OBJS=tokenizer.o halvolnyShell.o utils.o
LDFLAGS=
LIBS=

all:    halvolnyShell

$(SRCS):
	$(CC) $(CFLAGS) -c $*.c

halvolnyShell: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o halvolnyShell $(OBJS)

clean:
	rm -f *.o halvolnyShell

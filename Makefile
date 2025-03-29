CC		= gcc
CFLAGS	= -std=c11 -ggdb -O2 -Wall -Wextra -Wpedantic

all: expsh

expsh: expsh.c 
	$(CC) $(CFLAGS) -o $@ $^

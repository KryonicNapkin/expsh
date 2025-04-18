CC		= gcc
CFLAGS	= -std=c11 -ggdb3 -Wall -Wextra -Wpedantic

all: expsh

expsh: expsh.c 
	$(CC) $(CFLAGS) -lreadline -o $@ $^

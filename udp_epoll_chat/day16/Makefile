SRCS:=$(wildcard *.c)
TARGS:=$(SRCS:%.c=%)
CC:=gcc
all:$(TARGS)

%:%.c
	$(CC) -Wall $< -o $@
clean:
	rm -rf $(TARGS)

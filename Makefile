SRC = $(wildcard *.c)
BIN = $(patsubst %.c,%,$(SRC))
CFLAGS = -Wall -Wextra -g


all: $(BIN)

clean:
	rm -f $(BIN)
	rm -f *.o

.PHONY: all clean

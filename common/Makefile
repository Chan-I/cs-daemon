CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -I../include
LDFLAGS = 
SRC = utils.c
OBJ = $(SRC:.c=.o)
TARGET = utils.o

.PHONY: common clean

common: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

#!/bin/bash

include ../config.mk

SRC = server.c request_handler.c
OBJ = $(SRC:.c=.o)
TARGET = cs_daemon_server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(MAKE) -C pse
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ ../common/utils.o pse/*.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(MAKE) -C pse clean
	rm -f $(OBJ) $(TARGET)

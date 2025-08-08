include config.mk

.PHONY: all common server client clean

all: common server client

common:
	$(MAKE) -C common

server:
	$(MAKE) -C server

client:
	$(MAKE) -C client

clean:
	$(MAKE) -C common clean
	$(MAKE) -C server clean
	$(MAKE) -C client clean

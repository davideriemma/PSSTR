prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: main

thread.o: thread.c
	$(CC) $(LXRT_CFLAGS) -lpthread -c -o $@ $< $(LXRT_LDFLAGS)

main: main.c thread.o
	$(CC) $(LXRT_CFLAGS) -o $@ $^ $(LXRT_LDFLAGS)

clean:
	rm -f *.o thread main

.PHONY: clean

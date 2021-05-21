prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: rt_process

rt_process: main.c writer.o reader.o
	$(CC) $(LXRT_CFLAGS) -lpthread -o $@ $^ $(LXRT_LDFLAGS)

reader.o: reader.c
	$(CC) $(LXRT_CFLAGS) -c -o $@ $< $(LXRT_LDFLAGS)

writer.o: writer.c
	$(CC) $(LXRT_CFLAGS) -c -o $@ $< $(LXRT_LDFLAGS)

clean:
	rm -f *.o rt_process scope

.PHONY: clean

#ifndef HEADERS_H
#define HEADERS_H

#define STACK_SIZE 10000
#define SLEEP_TIME_S 1 

#define MAILBOX_ID "MyMailbox"
#define MAILBOX_SIZE 256

typedef struct{
	int req;
	int res;
}SharedMemory;

#endif

#ifndef MYMESSAGES_H
#define MYMESSAGES_H

#include <pthread.h>

#define SERVER_MESSAGES 2
#define CONNECT_REQUEST_MESSAGE  "1"
#define REPORT_MESSAGE  "3"

#define CLIENT_MESSAGES 3
#define START_MESSAGE  "2"
#define REPORT_MESSAGE  "3"
#define STOP_MESSAGE  "4"


static pthread_mutex_t lock = PTHREAD_COND_INITIALIZER;
static pthread_cond_t sendStart = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
   unsigned int quantity;
   unsigned int connected;
   unsigned int reported;
 }status;

extern status clientsStatuses;

char* readSocketLimiter(int fd, int BUFFER_SIZE, int* bytesRead);

char *readSocket(int fd, int BUFFER_SIZE, int sz_received, int* bytesRead);

int writeSocket(int fd, char* array, int BUFFER_SIZE, int sz_emit);

void *connectReqMessage (void *context);

void *startMessage (void *context);

void *reportMessage (void *context);

#endif

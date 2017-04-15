#ifndef MYMESSAGES_H
#define MYMESSAGES_H

#include <pthread.h>

#define PORT_DIST 5001

#define SERVER_MESSAGES 2
#define CONNECT_REQUEST_MESSAGE  "1"
#define REPORT_MESSAGE  "3"

#define CLIENT_MESSAGES 1
#define START_MESSAGE  "2"

#define LIMITER ';'

static pthread_mutex_t lock = PTHREAD_COND_INITIALIZER;
static pthread_cond_t sendStart = PTHREAD_MUTEX_INITIALIZER;

//TODO: Considerar remover el resultado final como un buffer y

typedef struct {
   int quantity;
   int connected;
   int reported;
 }status;

extern status clientsStatuses;

char* readSocketLimiter(int fd, int BUFFER_SIZE, int* bytesRead);

char *readSocket(int fd, int BUFFER_SIZE, int sz_received, int* bytesRead);

int writeSocket(int fd, char* array, int BUFFER_SIZE, int sz_emit);

void *connectReqMessage (void *context);

void *startMessage (void *context);

void *reportMessage (void *context);

#endif

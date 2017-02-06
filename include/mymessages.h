#ifndef MYMESSAGES_H
#define MYMESSAGES_H

#include <pthread.h>

typedef enum { false, true } bool;

#define PORT_DIST 5001

#define SERVER_MESSAGES 4
#define CONNECT_REQUEST_MESSAGE  "1"
#define CONNECT_ACK_MESSAGE  "3"
#define START_ACK_MESSAGE  "5"
#define REPORT_MESSAGE "6"

#define CLIENT_MESSAGES 2
#define CONNECT_REPLY_MESSAGE  "2"
#define START_MESSAGE "4"

static bool sendReport = false;


//TODO: Considerar remover el resultado final como un buffer y
//      enviar todos mensajes impreso por pontalla en forma de archivo
static struct report {
  int sock;
  const char *hostname; // this shouldnt be const char * ?
  const char *buffer;
 } reports[] = {
   { 0 , "", "" },
   { '\0' }
 };

static struct status {
   int quantity;
   int connected;
   int started;
   int reported;
 } clientsStatuses [] = {
   { -1, 0, 0, 0 },
   { '\0' }
 };


void *connectReqMessage (void *context);

void *connectRepMessage (void *context);

void *connectAckMessage (void *context);

void *startMessage (void *context);

void *startAckMessage (void *context);

void *reportMessage (void *context);

void *reportAckMessage (void *context);

#endif

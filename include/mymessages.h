#ifndef MYMESSAGES_H
#define MYMESSAGES_H

#include <pthread.h>

#define PORT_DIST 5001

#define SERVER_MESSAGES 4
#define CONNECT_REQUEST_MESSAGE  "1"
#define CONNECT_ACK_MESSAGE  "3"
#define START_ACK_MESSAGE  "5"
#define REPORT_MESSAGE "6"

#define CLIENT_MESSAGES 3
#define CONNECT_REPLY_MESSAGE  "2"
#define START_MESSAGE "4"
#define REPORT_ACK_MESSAGE "7"

static struct report {
  int sock;
  char const *hostname;
  char const *buffer;
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
   {   -1,   0,   0,   0 },
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

#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mymessages.h"

void *connectReqMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   //TODO: Write id of switch after CONNECT_REPLY_MESSAGE

   return NULL;
}

void *connectRepMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   //TODO: Write id of switch after CONNECT_REPLY_MESSAGE

   return NULL;
}

void *connectAckMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   //TODO: Dont do anything. Delete this function and create timeout management

   return NULL;
}

void *startMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   //TODO: Dont do anything. Delete this function and create timeout

   return NULL;
}

void *startAckMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   //TODO: Dont do anything. Delete this function and create timeout

   return NULL;
}

void *reportMessage (void *context) {
   int n;
   char buffer[256];
   bzero(buffer,256);
   struct report *reportParams;
   reportParams = &context;

   pthread_mutex_lock(&reportParams);
     n = read(reportParams->sock,buffer,255);

     if (n < 0) {
        perror("ERROR reading from socket of node slave");
        exit(1);
     }

     printf("Here is the message: %s\n",buffer);
     memcpy (reportParams->buffer, buffer, strlen(buffer)+1 );
     close(reportParams->sock);
   pthread_mutex_unlock(&reportParams);

   return NULL;
}

void *reportAckMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);

   return NULL;
}

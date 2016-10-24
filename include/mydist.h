#ifndef MYDIST_H
#define MYDIST_H

#include <pthread.h>

#define PORTREPORT 5001

static  struct report {
  int sock;
  char *hostname;
  char *buffer;
 } reports[] = {
   { 0 , "",		"" },
   { NULL }
 };


 void * serverSide(void *s);
 int clientSide(char *nodeMasterHostname, char* reportBuffer);
#endif

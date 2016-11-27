#ifndef MYSERVER_H
#define MYSERVER_H

static struct report {
  int sock;
  char *hostname;
  char *buffer;
 } reports[] = {
   { 0 , "", "" },
   { NULL }
 };

 static struct status {
   int quantity;
   int connected;
   int started;
   int reported;
 } clientsStatuses = {
   -1,
   0,
   0,
   0
 };


void * serverSide(void *s);

#endif

#ifndef MYREPORT_H
#define MYREPORT_H

typedef struct message {
  const char *buffer;
  message *next;
} message;


static struct report {
  int sock;
  const char *hostname;
  const char *buffer;
  message *list;
 } reports[] = {
   { 0 , "", "" },
   { '\0' }
 };

#endif

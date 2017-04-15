#ifndef MYREPORT_H
#define MYREPORT_H

typedef struct message {
  const char *buffer;
  struct message *next;
} message;


static struct report {
  int sock;
  const char *hostname;
  const char *buffer;
  struct message *list;
 } reports[] = {
   { 0 , "", "" },
   { '\0' }
 };

void displayMessages(struct report * myreport);
void enqueueMessage(char* item, struct report *myreport);
void dequeueMessage(struct report *myreport);

#endif

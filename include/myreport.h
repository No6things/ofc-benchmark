#ifndef MYREPORT_H
#define MYREPORT_H

typedef struct message {
  const char *buffer;
  struct message *next;
} message;


typedef struct{
  int sock;
  const char *hostname;
  const char *buffer;
  struct message *list;
 } report;

 extern report *reports;
 //TODO: hacer este puntero, una cola como message

void displayMessages(report *myreport);
void enqueueMessage(char* item, report *myreport);
void dequeueMessage(report *myreport);

#endif

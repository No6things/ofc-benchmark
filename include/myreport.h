#ifndef MYREPORT_H
#define MYREPORT_H

#define DELIMIT 1
#define LIMITER '*'
#define CSV_NEWLINE ';'
#define MAX_QUEUE 4
#define VALUES 0
#define AVGS 1
#define RESULTS 2
#define SNMP 3

typedef struct message {
  char *buffer;
  struct message *next;
  struct message *back;
} message;

typedef struct queue {
  int length;
  struct message *first;
  struct message *last;
} queue;

typedef struct report{
  int sock;
  char *hostname;
  struct queue queues[MAX_QUEUE];
 } report;

 extern report *myreport;
 extern report *reports;

void displayMessages(report *myreport, int id);
void enqueueMessage(char* item, report *myreport, int id,int delimit, int size);
#endif

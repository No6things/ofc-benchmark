#ifndef MYREPORT_H
#define MYREPORT_H

#define DELIMIT 1
#define LIMITER '*'
#define CSV_NEWLINE ';'

typedef struct message {
  char *buffer;
  struct message *next;
  struct message *back;
} message;


typedef struct report{
  int sock;
  int length;
  char *hostname;
  struct message *first;
  struct message *queue;
 } report;

 extern report *myreport;
 extern report *snmpReport;
 extern report *reports;

 /*
 TODO: Deberia modificar esta funcion para mostrar todos los reportes O
       crear un ciclo para mostrar todos los reportes*/
void displayMessages(report *myreport);
void enqueueMessage(char* item, report *myreport, int delimit);
void dequeueMessage(report *myreport);
char * parseReports();
#endif

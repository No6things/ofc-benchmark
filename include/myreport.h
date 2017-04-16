#ifndef MYREPORT_H
#define MYREPORT_H

#define LIMITER ';'

typedef struct message {
  char *buffer;
  struct message *next;
} message;


typedef struct{
  int sock;
  int length;
  char *hostname;
  struct message *list;
 } report;

 extern report *reports;
 //TODO: hacer este puntero, una cola como message


 /*
 TODO: Deberia modificar esta funcion para mostrar todos los reportes O
       crear un ciclo para mostrar todos los reportes*/
void displayMessages(report *myreport);
void enqueueMessage(char* item, report *myreport);
void dequeueMessage(report *myreport);

#endif

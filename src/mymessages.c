#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mymessages.h"

void *connectReqMessage (void *context) {
   char buffer[256];
   bzero(buffer,256);
   //TODO: Escribir mecanismo antes de enviar el mensaje
   //     que me permita esperar por los otros hilos
   //     evaluando una condicion con una variable global que represente
   //     la cantidad de clientes conectados y un tiempo maximo de espera
   //     antes de enviar el mensaje
   //     Candidato: Un loop infinito
   //TODO: Write START_MESSAGE
   //TODO: Make SNMP queries frequently during the test with:
   //      asynchronousSnmp(params->controllerHostname);
   return NULL;
}

void *startMessage (void *context) {

   //TODO: Write START_ACK_MESSAGE and call benchmark function
   //TODO: Considerar que se debe retornar el reporte del benchmark para
   //     poder enviar el mensaje REPORT_MESSAGE
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
   //TODO: considerar almacenar la informacion en un archivo o varios archivos
   //TODO: Enviar mensaje REPORT_ACK_MESSAGE

   return NULL;
}

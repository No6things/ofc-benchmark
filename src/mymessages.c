#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <string.h>

#include "../include/mymessages.h"

static struct status *clients;

char* readSocket(int fd, int BUFFER_SIZE, int sz_received, int* bytesRead)
{
  int i = 0, sz = 0, rt = 0, count=0;
  char *array = (char *)malloc(BUFFER_SIZE);
  memset(array, 0, BUFFER_SIZE);
  fcntl(fd, F_SETFL, O_NONBLOCK);
  for (i = 0; i < BUFFER_SIZE; i += sz_received)
  {
    while(sz_received-sz)
    {
      rt = read(fd, array + i + sz, sz_received-sz);
      if(rt < 1)
      {
        if (errno == EAGAIN) {
            fcntl(fd, F_SETFL, 0);
            *bytesRead = count;
            return array;
        } else {
            perror("readSocket");
            exit(1);
        }
      }
      sz += rt;
      count += sz;
    }
    sz = 0;
  }
  fcntl(fd, F_SETFL, 0);
  *bytesRead = count;
  return array;
}

void *connectReqMessage (void *context) {
  char buffer[256];
  bzero(buffer,256);
  clients = clientsStatuses;

  pthread_mutex_lock(&lock);
  while(clients->connected != clients->quantity){
    printf("Blocked slave id: %d.\n", clients->connected);
    pthread_cond_wait(&sendStart, &lock);
  }
  printf("Imaginary implementing write to slave node\n");
  pthread_mutex_unlock(&lock);

  //TODO: Escribir mecanismo antes de enviar el mensaje
  //     que me permita esperar por los otros hilos
  //     evaluando una condicion con una variable global que represente
  //     la cantidad de clientes conectados y un tiempo maximo de espera
  //     antes de enviar el mensaje
  //     Candidato: Un loop infinito
  //TODO: Write START_MESSAGE
  //TODO: Make SNMP queries frequently during the test with:
  //      asynchronousSnmp(params->controllerHostname);
  pthread_exit(NULL);
}

void *startMessage (void *context) {

   //TODO: Write START_ACK_MESSAGE and call benchmark function
   //TODO: Considerar que se debe retornar el reporte del benchmark para
   //     poder enviar el mensaje REPORT_MESSAGE
   pthread_exit(NULL);
}

void *reportMessage (void *context) {
   int n;
   char buffer[256];
   bzero(buffer,256);
   struct report *reportParams;
   reportParams = &context;

   //pthread_mutex_lock(&reportParams);
     n = read(reportParams->sock,buffer,255);

     if (n < 0) {
        perror("ERROR reading from socket of node slave");
        exit(1);
     }

     printf("Report received: %s\n",buffer);
     memcpy (reportParams->buffer, buffer, strlen(buffer)+1 );
     close(reportParams->sock);
  // pthread_mutex_unlock(&reportParams);
   //TODO: considerar almacenar la informacion en un archivo o varios archivos
   //TODO: Enviar mensaje REPORT_ACK_MESSAGE

   pthread_exit(NULL);
}

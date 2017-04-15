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

#include "../include/myreport.h"
#include "../include/mymessages.h"

status clientsStatuses;

char* readSocket(int fd, int BUFFER_SIZE, int sz_received, int* bytesRead)
{
  int i = 0, sz = 0, rt = 0, count = 0;
  char *array = (char *)malloc(BUFFER_SIZE);
  memset(array, 0, BUFFER_SIZE);
  for (i = 0; i < BUFFER_SIZE; i += sz_received)
  {
    while(sz_received - sz)
    {
      printf("sz_received-sz = %d\n", sz_received-sz);
      rt = read(fd, array + i + sz, sz_received-sz);
      if(rt < 1)
      {
        if (rt == 0) {
          printf("count while reading = 0\n");
        }
        if (errno != EAGAIN) {
          perror("readSocket");
          exit(1);
        }
      }
      sz += rt;
      count += sz;
    }
    sz = 0;
  }
  *bytesRead = count;
  printf("Read %d byte(s), trough socket file descriptor %d  the content '%s' with length %zu \n", *bytesRead, fd, array, strlen(array));
  return array;
}

int writeSocket(int fd, char* array, int BUFFER_SIZE, int sz_emit)
{
  int i = 0, sz = 0, written = 0;
  for(i = 0; i < BUFFER_SIZE; i += sz_emit )
  {
      while(sz_emit - sz)
      {
        written = write(fd, array + i + sz, sz_emit - sz);
        if (written == -1) {
          perror("writeSocket");
          exit(1);
        } else {
          sz += written;
        }
      }
      sz = 0;
  }
  printf("Sent %d byte(s), trough socket file descriptor %d  the content '%s' with length %zu \n", written, fd, array, strlen(array));
  return i;
}

void *connectReqMessage (void *context) {
  int clientFd = *((int*)context);
  int bytesWritten;
  char * buffer;
  buffer = (char *)malloc(strlen(START_MESSAGE) + 1);
  snprintf(buffer, strlen(START_MESSAGE) + 1, START_MESSAGE);

  printf("connected mymsg: %d, total: %d\n", clientsStatuses.connected, clientsStatuses.quantity);
  pthread_mutex_lock(&lock);
  while(clientsStatuses.connected < clientsStatuses.quantity){
    //TODO: Agregar un timeout para enviar el mensaje aun si no estan todos conectados
    printf("Blocked slave id: %d. of %d\n", clientsStatuses.connected, clientsStatuses.quantity);
    pthread_cond_wait(&sendStart, &lock);
  }
  pthread_mutex_unlock(&lock);

  printf("about to write %s\n", buffer);

  bytesWritten = writeSocket(clientFd, buffer, 2, 1);
  if (bytesWritten < 0) {
    perror("connectReqMessage");
    exit(0);
  }
  pthread_exit(NULL);
}


void *reportMessage (void *context) {
   int n;
   char buffer[256];
   bzero(buffer,256);
   report *reportParams;
   reportParams = (report *)context;

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

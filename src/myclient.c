#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string.h>

#include "../include/mymessages.h"
#include "../include/benchmark.h"
#include "../include/myclient.h"
/*
void enqueue(char* item)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->data = item;
    nptr->next = NULL;
    if (rear == NULL)
    {
        front = nptr;
        rear = nptr;
    }
    else
    {
        rear->next = nptr;
        rear = rear->next;
    }
}

void display()
{
    struct node *temp;
    temp = front;
    printf("\n");
    while (temp != NULL)
    {
        printf("%s\t", temp->data);
        temp = temp->next;
    }
}

void dequeue()
{
    if (front == NULL)
    {
        printf("\n\nqueue is empty \n");
    }
    else
    {
        struct node *temp;
        temp = front;
        front = front->next;
        printf("\n\n%d deleted", temp->data);
        free(temp);
    }
}
*/
int writeSocket(int fd, char* array, int SIZE, int sz_emit)
{
  //#######################
  //    server code
  //#######################
  int i=0, sz=0, written = 0;
  for(i = 0; i < SIZE; i += sz_emit )
  {
      while(sz_emit-sz)
      {
        written=write(fd, array+i+sz, sz_emit-sz);
        if (written == -1) {
          perror("writeToClient -1\n");
        } else {
          sz += written;
        }
      }
      sz = 0;
  }
  return i;
}


int clientSide(const char *nodeMasterHostname) {
   int serverFd, portno, bytes, end;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   //struct inputValues *params = &benchmarkArgs; TODO: UNUSED
   //char *reportsBuffer; TODO: UNUSED

   /* Initializing*/
   char buffer[BUFSIZ];
   bzero(buffer, BUFSIZ);
   memcpy (buffer, CONNECT_REQUEST_MESSAGE, strlen(CONNECT_REQUEST_MESSAGE) + 1);
   portno = 5101;

   /* Opening Socket */
   if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   /* Validating Server existence*/
   server = gethostbyname(nodeMasterHostname);
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   /* Initializing Internet structures*/
   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(portno);

   /* Establishing server connection*/
   printf("Establishing connection with %s at port %d\n", nodeMasterHostname, portno);
   if (connect(serverFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   } else {
     printf("Connected succesfuly through socket file descriptor %d\n", serverFd);
   }

   /*Sending CONNECT_REQUEST_MESSAGE to the server */
   printf("Sending CONNECT_REQUEST_MESSAGE: %s.\n", buffer);
   bytes = write(serverFd, buffer, strlen(buffer));
   if (bytes < 0) {
      perror("ERROR writing to socket");
      exit(1);
   } else {
     printf("Sent %d byte(s), trough socket file descriptor %d  the content '%s' with length %zu \n", bytes, serverFd, buffer, strlen(buffer));
   }

   /* Reinitilize buffer*/
   bzero(buffer,256);
   end = 0;
   while (1) {
     /*Read server response*/
     bytes = read(serverFd, buffer, 1);
     printf("buffer");
     if (bytes < 0) {
       perror("ERROR reading message from server");
       exit(1);
     }

     if (strcmp(buffer, START_MESSAGE) == 0) {
       end = 1;
       printf("received START_MESSAGE\n");
       /*
       TODO: Recibir arreglo de reportes de controllerBenchmarking()
       TODO: Enviar mensaje REPORT_MESSAGE
       */
     } else {
       printf("Message received: '%s'\n",buffer);
       perror("ERROR unknown message header from server");
       exit(1);
     }
     if (end) break;
  }

  return 0;
}

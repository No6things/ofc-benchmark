#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string.h>

#include "../include/myreport.h"
#include "../include/mymessages.h"
#include "../include/benchmark.h"
#include "../include/myclient.h"

int clientSide(const char *nodeMasterHostname) {
   int serverFd, portno, bytes, end;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   //char *reportsBuffer; TODO: UNUSED

   /* Initializing*/
   char * buffer;
   buffer = (char *)malloc(strlen(CONNECT_REQUEST_MESSAGE) + 1);
   snprintf(buffer, strlen(CONNECT_REQUEST_MESSAGE) + 1, CONNECT_REQUEST_MESSAGE);
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
   bytes = writeSocket(serverFd, buffer, 2, 1);

   /* Reinitilize buffer*/
   end = 0;
   while (1) {
     bytes = 0;
     char * buffer2;
     buffer2 = NULL;
     /*Read server response*/
     buffer2 = readSocket(serverFd, 1, 1, &bytes);

     if (strcmp(buffer2, START_MESSAGE) == 0) {
       end = 1;
       printf("received START_MESSAGE\n");
       controllerBenchmarking();
       displayMessages(reports);
       /*
       TODO: Deberia modificar esta funcion para mostrar todos los reportes O
             crear un ciclo para mostrar todos los reportes
       TODO: Enviar mensaje REPORT_MESSAGE
       */
     } else {
       printf("received: '%s'\n",buffer2);
       perror("Uknown message for distributed mode");
       exit(1);
     }
     if (end) break;
  }

  return 0;
}

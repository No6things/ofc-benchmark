#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mymessages.h"
#include "../include/benchmark.h"
#include "../include/myclient.h"

int clientSide(const char *nodeMasterHostname) {
   int sockfd, portno, n, end;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   struct inputValues *params = &benchmarkArgs;
   char *reportsBuffer;

   char buffer[BUFSIZ];
   portno = PORT_DIST;

   /* CCONNECTION */
   printf("Establishing connection with %s at port %d\n", nodeMasterHostname, portno);
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   } else {
     printf("Creation of socket sucessful: %d\n", sockfd);
   }

   server = gethostbyname(nodeMasterHostname);

   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   } else {
     printf("Lookup of server hostname successful\n" );
   }
   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   printf("Preparing to connect");
   printf("q\n");

   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      printf("yikes");
      perror("ERROR connecting");
      exit(1);
   } else {
     printf("Connected succesfuly through socket file descriptor %d\n", sockfd);
   }

   bzero(buffer, BUFSIZ);
   memcpy (buffer, CONNECT_REQUEST_MESSAGE, strlen(CONNECT_REQUEST_MESSAGE) + 1);
   printf("Sending CONNECT_REQUEST_MESSAGE: %s", buffer);
   //   memcpy (buffer, reportBuffer, strlen(reportBuffer)+1);
   /*   Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));
   bzero(buffer,256);

   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }

   end = 0;

   while (1) {
     n = read(sockfd, buffer, 1);
     printf("read: %s.\n",buffer);
     if (n < 0) {
       perror("ERROR reading message from server");
       exit(1);
     }

     if (strcmp(buffer, START_MESSAGE) == 0) {
       end = 1;
       /*
       TODO: Recibir arreglo de reportes de controllerBenchmarking()
       TODO: Enviar mensaje REPORT_MESSAGE
       */
     } else {
       printf("Message received: '%s'\n",buffer);
       perror("ERROR unknown message header from server");
     }
     if (end) break;
  }

  return 0;
}

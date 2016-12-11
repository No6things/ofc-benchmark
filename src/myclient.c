#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mymessages.h"
#include "../include/myclient.h"

int clientSide(char *nodeMasterHostname, char *reportBuffer) {
   int sockfd, portno, n, end;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[BUFSIZ];

   portno = PORT_DIST;

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(nodeMasterHostname);

   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   bzero(buffer,BUFSIZ);
   memcpy (buffer, CONNECT_REQUEST_MESSAGE, strlen(CONNECT_REQUEST_MESSAGE)+1);
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
   while (1){
     n = read(sockfd, buffer, 1);
     printf("read: %s.\n",buffer);
     if (n < 0) {
       perror("ERROR reading message from server");
       exit(1);
     }

     if (strcmp(buffer, CONNECT_REPLY_MESSAGE) == 0) {
       /*bzero(buffer,256);
       memcpy (buffer, CONNECT_REPLY_MESSAGE, strlen(CONNECT_REPLY_MESSAGE)+1);
       n = write(sockfd, buffer, strlen(buffer));
       bzero(buffer,256);

       if (n < 0) {
          perror("ERROR writing to socket");
          exit(1);
       }*/
     } else if (strcmp(buffer, START_MESSAGE) == 0) {
       /*bzero(buffer,256);
       memcpy (buffer, CONNECT_REPLY_MESSAGE, strlen(CONNECT_REPLY_MESSAGE)+1);
       n = write(sockfd, buffer, strlen(buffer));
       bzero(buffer,256);

       if (n < 0) {
          perror("ERROR writing to socket");
          exit(1);
       }*/
     } else if (strcmp(buffer, REPORT_ACK_MESSAGE) == 0) {
       end = 1;
     } else {
       printf("Message received: '%s'\n",buffer);
       perror("ERROR unknown message header from server");
     }
     if (end) break;
  }
  return 0;
}
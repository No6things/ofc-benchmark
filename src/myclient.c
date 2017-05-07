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
   struct message *temp;
   char *buffer = (char *)malloc(strlen(CONNECT_REQUEST_MESSAGE) + 1);


   /* Initializing*/
   snprintf(buffer, strlen(CONNECT_REQUEST_MESSAGE) + 1, CONNECT_REQUEST_MESSAGE);
   portno = 5101;
   end = 0;

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

   printf("Establishing connection with %s at port %d\n", nodeMasterHostname, portno);
   if (connect(serverFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   } else {
     printf("Connected succesfuly through socket file descriptor %d\n", serverFd);
   }

   printf("Sending CONNECT_REQUEST_MESSAGE: %s.\n", buffer);
   bytes = writeSocket(serverFd, buffer, 2, 1);

   while (1) {
     bytes = 0;

     buffer = readSocket(serverFd, 1, 1, &bytes);

     if (strcmp(buffer, START_MESSAGE) == 0) {

       printf("received START_MESSAGE\n");
       controllerBenchmarking();
       displayMessages(myreport, VALUES);
       displayMessages(myreport, AVGS);
       displayMessages(myreport, RESULTS);

       printf("sending REPORT_MESSAGE\n");
       snprintf(buffer, strlen(REPORT_MESSAGE) + 1, REPORT_MESSAGE);
       writeSocket(serverFd, buffer, 2 , 1);

       //Sending Values
       printf("VALUES\n");
       int repId = 0;
       temp = myreport->queues[VALUES].first;
       while (temp != NULL) {
         bytes = writeSocket(serverFd, temp->buffer, strlen(temp->buffer), strlen(temp->buffer));
         temp = temp->back;
         repId++;
         printf("%d ", repId);
       }

       //Sending Partials
       printf("\nPARTIALS\n");
       repId = 0;
       temp = myreport->queues[AVGS].first;
       while (temp != NULL) {
         bytes = writeSocket(serverFd, temp->buffer, strlen(temp->buffer), strlen(temp->buffer));
         temp = temp->back;
         repId++;
         printf("%d ", repId);
       }

       //Sending Results
       printf("\nRESULTS\n");
       repId = 0;
       temp = myreport->queues[RESULTS].first;
       while (temp != NULL) {
         bytes = writeSocket(serverFd, temp->buffer, strlen(temp->buffer), strlen(temp->buffer));
         temp = temp->back;
         repId++;
         printf("%d ", repId);
       }
       //free(myreport);
     } else if (strcmp(buffer, STOP_MESSAGE) == 0) {
       printf("End of transmission\n");
       break;
     } else {
       printf("Received: '%s'\n",buffer);
       perror("Uknown message for distributed mode");
       exit(1);
     }
     buffer = NULL;
  }

  shutdown(serverFd, SHUT_WR);
  return 0;
}

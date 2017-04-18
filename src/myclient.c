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
       displayMessages(myreport);

       snprintf(buffer, strlen(REPORT_MESSAGE) + 1, REPORT_MESSAGE);
       writeSocket(serverFd, buffer, 2 , 1);
       printf("sent REPORT_MESSAGE\n");

       struct message *temp;
       char * listLength;
       int index = 0;
       temp = myreport->list;
       listLength = (char *)malloc(6 + 1); //5 digits + delimeter + null
       snprintf(listLength, 6, "%d%c", myreport->length, LIMITER);
       writeSocket(serverFd, listLength, strlen(listLength) + 1 , strlen(listLength));
       printf("sent list length %d\n", myreport->length);

       while (temp != NULL)
       {
         bytes = writeSocket(serverFd, temp->buffer, strlen(temp->buffer), strlen(temp->buffer));
         temp = temp->next;
         index++;
       }
       printf("End of transmission\n");
     } else {
       printf("received: '%s'\n",buffer2);
       perror("Uknown message for distributed mode");
       exit(1);
     }
     if (end) break;
  }
  //TODO: Replace this way to ensure data sending, at least remove this for and
  //      remove end = 1 breaking sentence
  for (;;){
  }
  shutdown(serverFd, SHUT_WR);
  return 0;
}

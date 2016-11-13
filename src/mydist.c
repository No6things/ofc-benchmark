#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mydist.h"

static struct report *rp;

  void *clientManagement (void *context) {
     int n;
     char buffer[256];
     bzero(buffer,256);
     struct report *reportParams, *reportBlocked;
     reportParams = &context;

     pthread_mutex_lock(&reportParams);
       n = read(reportParams->sock,buffer,255);

       if (n < 0) {
          perror("ERROR reading from socket of node slave");
          exit(1);
       }

       sprintf("Here is the message: %s\n",buffer);
       memcpy (reportParams->buffer, buffer, strlen(buffer)+1 );
       close(reportParams->sock);
     pthread_mutex_unlock(&reportParams);

     return NULL;
  }

  void * serverSide(void *s) {
     int sockfd, newsockfd, portno, clilen, nNodes;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n, threadErr, nodesConnected;

     nNodes = *((int *) s);
     pthread_t nodesThreads [nNodes];
     nodesConnected= 0;
     rp = reports;


     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(PORTREPORT);

     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
     }

     /* Now start listening for the clients, here
        * process will go in sleep mode and will wait
        * for the incoming connection
     */

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
           perror("ERROR on accept");
           exit(1);
        }

        memcpy (rp->sock, &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr));
        memcpy (rp->hostname, &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr));

        /* Create thread */

        threadErr= pthread_create(&nodesThreads[nodesConnected], NULL, &clientManagement, rp);

        if(threadErr){
          pthread_join(&nodesThreads[nodesConnected], NULL);
          perror("ERROR on node thread");
          exit(1);
        }

        if (nodesConnected == nNodes) break;
        rp++;
        nodesConnected++;
     }
     for (n=nodesConnected;n>=0;n--)
      pthread_join(&nodesThreads[n], NULL);

  }

  int clientSide(char *nodeMasterHostname, char* reportBuffer) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[256];

   portno = PORTREPORT;

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

   printf("Storing message: ");
   bzero(buffer,256);
   memcpy (buffer, reportBuffer, strlen(reportBuffer)+1 );

   /* Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));

   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   return 0;
}

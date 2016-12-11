#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "../include/mymessages.h"
#include "../include/myserver.h"

static struct report *rp;
static struct status *clients;

void * serverSide(void *s) {
   socklen_t sockfd, newsockfd;
   unsigned int n, iThreads, nThreads, threadErr, clilen;
   char * buffer;
  // bzero(buffer,256);
   struct sockaddr_in serv_addr, cli_addr;

   rp = reports;
   clients = clientsStatuses;
   clients->quantity = *((int *) s) - 1;
   iThreads = 0;
   nThreads = clients->quantity * SERVER_MESSAGES;
   pthread_t nodesThreads [nThreads];

   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(PORT_DIST);

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

      n = read(newsockfd, buffer, 1);
      printf("String, %s.\n", buffer);

      if (n < 0) {
        perror("ERROR reading message from slave node");
        exit(1);
      }

      if (strcmp(buffer, CONNECT_REQUEST_MESSAGE) == 0) {
          printf("Switch condition is good\n");
          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &connectReqMessage, rp);

          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating CONNECT_REQUEST_MESSAGE  thread");
            exit(1);
          }

          iThreads++;

      } else if(strcmp(buffer, CONNECT_ACK_MESSAGE) == 0) {
          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &connectAckMessage, rp);

          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating CONNECT_ACK_MESSAGE  thread");
            exit(1);
          }

          clients->connected++;
          iThreads++;

      } else if(strcmp(buffer, START_ACK_MESSAGE) == 0) {
          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &startAckMessage, rp);

          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating START_ACK_MESSAGE  thread");
            exit(1);
          }

          clients->started++;
          iThreads++;

      } else if(strcmp(buffer, REPORT_MESSAGE) == 0) {
          rp->sock = newsockfd;
          printf("socket: %d, rp->sock: %d\n", newsockfd, rp->sock);
          memcpy (rp->hostname, &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr));
          /* Create thread */

          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &reportMessage, rp);

          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating REPORT_MESSAGE thread");
            exit(1);
          }

          rp++;
          clients->reported++;

      } else {
          printf("Message received: '%s'\n",buffer);
          perror("ERROR unknown message header from slave node");
      }
      if (clients->reported == clients->quantity) break;
   }
   for (n = nThreads; n >= 0; n--)
    pthread_join(nodesThreads[n], NULL);
    return 0;
}

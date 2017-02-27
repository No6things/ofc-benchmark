#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <string.h>

#include "../include/mymessages.h"
#include "../include/myserver.h"

static struct report *rp;
static struct status *clients;

void * serverSide(unsigned int s) {
   char buffer[1024];
   memset(buffer, '0',sizeof(buffer));

   socklen_t sockfd, newsockfd;
   unsigned int  iThreads, nThreads, threadErr, clilen;
   int n;
   struct sockaddr_in serv_addr, cli_addr;

   rp = reports;
   clients = clientsStatuses;
   clients->quantity = s - 1;
   clients->connected = 0;

   iThreads = 0;
   nThreads = clients->quantity * SERVER_MESSAGES;
   pthread_t nodesThreads [nThreads];


   printf("The network will have %d node slaves\n", clients->quantity);


   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   memset((char *) &serv_addr, 0, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(5101);

   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }

   /* Now start listening for the clients, here
    * process will go in sleep mode and will wait
    * for the incoming connection
   */

   if (listen(sockfd, 5) < 0) {
     perror("ERROR listening");
     exit(1);
   }


   clilen = sizeof(cli_addr);

   while (1) {

      if ((newsockfd = accept(sockfd, (struct sockaddr *)NULL, NULL)) < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      printf("Client connected with socket %d.\n", newsockfd);

      /*ioctl(newsockfd, FIONREAD, &n);
      if (n > 0) {
        printf("ioctl %d\n", n);

      }*/
      while ((n = read(newsockfd, buffer, sizeof(buffer)-1)) > 0)
      {
          buffer[n] = 0;
          if(fputs(buffer, stdout) == EOF)
          {
            printf("Error : Fputs error\n");
          } else {
            printf("There is more to read, but now we have %s\n", buffer);
          }
      }
      //TODO: Considerar que contiene el buffer cuando se recibieron simultaneamente
      // multiples mensajes previo a la lectura
      printf("Message received, '%s' of lenght %d.\n", buffer, n);

      if (n < 0) {
        perror("ERROR reading message from slave node");
        exit(1);
      }
      if (strcmp(buffer, CONNECT_REQUEST_MESSAGE) == 0) {
          printf("Some node wrote us by a CONNECT_REQUEST_MESSAGE\n");

          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &connectReqMessage, rp);
          //TODO: Considerar que no todos las funciones de mensajes necesitan
          // el objecto report. Sin embargo siempre se debe enviar el socket para
          // responder el mensaje.
          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating CONNECT_REQUEST_MESSAGE  thread");
            exit(1);
          } else {
            iThreads++;
          }
          pthread_mutex_lock(&lock);
            clients->connected++;
            if (clients->connected == clients->quantity) {
              pthread_cond_broadcast(&sendStart);
            }
          pthread_mutex_unlock(&lock);

      } else if (strcmp(buffer, REPORT_MESSAGE) == 0) {
          printf("Some node wrote us by a REPORT_MESSAGE\n");

          clients->reported++;

          //delete condition sendStart
          rp->sock = newsockfd;
          printf("socket: %d, rp->sock: %d\n", newsockfd, rp->sock);
          memcpy (rp->hostname, &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr));

          //TODO: Considerar remover el hilo y manejar la recepcion de reportes de manera
          //      secuencial
          threadErr= pthread_create(&nodesThreads[iThreads], NULL, &reportMessage, rp);

          if(threadErr){
            pthread_join(nodesThreads[iThreads], NULL);
            perror("ERROR creating REPORT_MESSAGE thread");
            exit(1);
          } else {
            rp++;
          }

      } else {
          printf("Message received: '%s'\n",buffer);
          perror("ERROR unknown message header from slave node");
      }
      if (clients->reported == clients->quantity) break;
      //   TODO: Considerar que la cantidad de hilos en iThreads se corresponda a
      //        la cantidad de hilos que debieron haberse usado

   }
   for (n = nThreads; n >= 0; n--)
      pthread_join(nodesThreads[n], NULL);

   pthread_mutex_destroy(&lock);
   pthread_cond_destroy(&sendStart);
   return 0;
}

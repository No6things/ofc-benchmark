#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string.h>

#include "../include/myreport.h"
#include "../include/mymessages.h"
#include "../include/myserver.h"

void * serverSide(unsigned int s) {
   unsigned int  iThreads, nThreads, threadErr;
   socklen_t serverFd, clientFd;
   struct sockaddr_in serv_addr, cli_addr;
   reports = (report *)malloc(sizeof(*reports));

   int n;
   socklen_t clilen;

   // Initializing variables
   clilen = sizeof(cli_addr);

   clientsStatuses.quantity = s - 1;
   clientsStatuses.connected = 0;
   clientsStatuses.reported = 0;

   iThreads = 0;
   nThreads = clientsStatuses.quantity;
   pthread_t nodesThreads [nThreads];

   printf("The network will have %d node slaves\n", clientsStatuses.quantity);

   // Opening Socket
   if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   // Initializing server Internet structures
   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   printf("SERVER_PORT %d\n",SERVER_PORT);
   serv_addr.sin_port = htons(SERVER_PORT);

   //Binding server socket
   if (bind(serverFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR binding socket");
      exit(1);
   }

   //Listening for clientsStatuses
   if (listen(serverFd, 5) < 0) {
      perror("ERROR listening socket");
      exit(1);
   }
   while (1) {
      // Accept request connections
      if ((clientFd = accept(serverFd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
         perror("ERROR accepting slave");
         exit(1);
      }

      pthread_mutex_lock(&lock);
        reports->hostname = inet_ntoa(cli_addr.sin_addr);
      pthread_mutex_unlock(&lock);

      printf("Client connected with socket %d and hostname: %s\n", clientFd, reports->hostname);
      threadErr = pthread_create(&nodesThreads[iThreads], NULL, &clientManagement, &clientFd);

      if (threadErr) {
        pthread_join(nodesThreads[iThreads], NULL);
        perror("Creating clientManagement thread");
        exit(1);
      }
      iThreads++;

      if (clientsStatuses.reported == clientsStatuses.quantity) break;
   }
   for (n = nThreads; n >= 0; n--)
      pthread_join(nodesThreads[n], NULL);

   pthread_mutex_destroy(&lock);
   pthread_cond_destroy(&sendStart);
   return 0;
}


void *clientManagement(void *context) {
  char *buffer;
  int clientFd = 0, bytesRead = 0, nLines = 0, index = 0, messageReceived = 0;
  unsigned int threadErr;
  pthread_t messageThread;

  clientFd = *((int*)context);
  buffer = NULL;
  while (1) {
    buffer = readSocket(clientFd, 2, 1, &bytesRead);

    if (strcmp(buffer, CONNECT_REQUEST_MESSAGE) == 0) {
        printf("CONNECT_REQUEST_MESSAGE\n");

        pthread_mutex_lock(&lock);
          clientsStatuses.connected++;
          printf("Connected: %d/%d\n", clientsStatuses.connected, clientsStatuses.quantity);
          if (clientsStatuses.connected == clientsStatuses.quantity) {
            printf("Broadcasting 'send START_MESSAGE' order\n");
            pthread_cond_broadcast(&sendStart);
          }
          while(clientsStatuses.connected < clientsStatuses.quantity){
            //TODO: Agregar un timeout para enviar el mensaje aun si no estan todos conectados
            printf("Blocked slave id: %d. of %d\n", clientsStatuses.connected, clientsStatuses.quantity);
            pthread_cond_wait(&sendStart, &lock);
          }
        pthread_mutex_unlock(&lock);

        snprintf(buffer, strlen(START_MESSAGE) + 1, START_MESSAGE);
        bytesRead = writeSocket(clientFd, buffer, 2, 1);
        if (bytesRead < 0) {
          perror("connectReqMessage");
          exit(0);
        }
        messageReceived++;
    } else if (strcmp(buffer, REPORT_MESSAGE) == 0) {
        printf("REPORT_MESSAGE\n");

        pthread_mutex_lock(&lock);
          clientsStatuses.reported++;
          printf("Reported: %d/%d\n", clientsStatuses.reported, clientsStatuses.quantity);
          reports->sock = clientFd;
        pthread_mutex_unlock(&lock);

        buffer = readSocketLimiter(clientFd, 5, &bytesRead);
        nLines = atoi(buffer);
        printf("Read list length %s\n", buffer);
        buffer = (char *)malloc(150 + 1);

        while (index < nLines) {
          bytesRead = 0;
          buffer = readSocketLimiter(clientFd, 150, &bytesRead);
          printf("%s\n", buffer);
          //enqueueMessage(buffer, reports);
          index++;
        }

        if (bytesRead > 0) {
          pthread_mutex_lock(&lock);
            printf("Offsetting report\n");
            reports++;
          pthread_mutex_unlock(&lock);
          messageReceived++;
        }
    } else {
        perror("ERROR unknown message from node");
    }
    if (messageReceived == SERVER_MESSAGES) break;
  }

  pthread_exit(NULL);
}

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string.h>

#include "../include/mysnmp.h"
#include "../include/myreport.h"
#include "../include/mymessages.h"
#include "../include/myplot.h"
#include "../include/myserver.h"

void * serverSide(unsigned int s) {
   int n;
   unsigned int index, nThreads, threadErr, param;
   socklen_t serverFd, clientFd, clilen;
   struct sockaddr_in serv_addr, cli_addr;

   // Initializing variables
   clilen = sizeof(cli_addr);

   clientsStatuses.quantity = s - 1;
   clientsStatuses.connected = 0;
   clientsStatuses.reported = 0;

   reports = (struct report *)malloc(clientsStatuses.quantity * sizeof(struct report));

   index = 0;
   nThreads = clientsStatuses.quantity;
   pthread_t nodesThreads[nThreads];

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

      reports[index].sock = clientFd;
      reports[index].hostname = inet_ntoa(cli_addr.sin_addr);

      printf("Client connected with socket %d and hostname: %s\n", reports[index].sock, reports[index].hostname);
      param = index;
      threadErr = pthread_create(&nodesThreads[index], NULL, &clientManagement, &param);
      if (threadErr) {
        pthread_join(nodesThreads[index], NULL);
        perror("Creating clientManagement thread");
        exit(1);
      }
      index++;

      if (clientsStatuses.reported == clientsStatuses.quantity) break;
   }
   snmpStop = 1;
   for (n = nThreads; n >= 0; n--)
      pthread_join(nodesThreads[n], NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&sendStart);
    return 0;
}


void *clientManagement(void *context) {
  char *buffer;
  int clientFd, threadErr, bytesRead, nLines, index, messageReceived, id, nSwitches,
  testRange, mode;
  pthread_t snmp_thread;

  //Initializing variables
  threadErr = 0;
  bytesRead = 0;
  nSwitches = 0;
  nLines = 0;
  index = 0;
  mode = 0;
  testRange = 0;
  messageReceived = 0;
  id = *((int *)context);
  clientFd = reports[id].sock;
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
            threadErr = pthread_create(&snmp_thread, NULL, &asynchronousSnmp, NULL);
            if (threadErr) {
              pthread_join(snmp_thread, NULL);
              perror("Creating SNMP thread");
              exit(1);
            }
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
        pthread_mutex_unlock(&lock);

        printf("******** RESULTS *********\n");
        //REPORT ENVIRONMENT
        buffer = readSocketLimiter(clientFd, 5, &bytesRead);
        nSwitches = atoi(buffer);
        printf("nSwitches: %d\n", nSwitches);
        buffer = NULL;
        buffer = readSocketLimiter(clientFd, 5, &bytesRead);
        nLines = atoi(buffer);
        printf("nLines/loops: %d\n", nLines);
        buffer = NULL;
        buffer = readSocketLimiter(clientFd, 5, &bytesRead);
        mode = atoi(buffer);
        printf("mode: %d\n", mode);
        buffer = NULL;
        buffer = readSocketLimiter(clientFd, 5, &bytesRead);
        testRange = atoi(buffer);
        printf("testRange: %d\n", testRange);

        //REPORT DATA
        bytesRead = plotManagement(clientFd, id, nSwitches, nLines, mode, testRange);

        if (bytesRead > 0) messageReceived++;
    } else {
        perror("ERROR unknown message from node");
    }
    if (messageReceived == SERVER_MESSAGES) break;
  }
  pthread_join(snmp_thread, NULL);
  displayMessages(myreport, 2);
  pthread_exit(NULL);
}

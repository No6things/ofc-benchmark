#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <string.h>

#include "../include/myreport.h"
#include "../include/mymessages.h"
#include "../include/myserver.h"

static struct report *rp;

//in @param
//@param fd is the file descriptor of the socket to read from
//@param SIZE the size of datas you want to read from the socket
//@param sz_received the size of byte to read in one loop step
//@param length, the length of data received
//@param read_err if 0 no error if -1 an error occurs use errno from #include <errno.h> to know more about that error
//out @param
//a pointer to an array of size SIZE containing the data readed

void * serverSide(unsigned int s) {
   unsigned int  iThreads, nThreads, threadErr;
   socklen_t serverFd, clientFd;
   struct sockaddr_in serv_addr, cli_addr;
   char* buffer;
   int n, bytesRead, portno, BUFFER_SIZE = 100;

   // Initializing variables
   rp = reports;
   clientsStatuses.quantity = s - 1;
   clientsStatuses.connected = 0;
   nThreads = clientsStatuses.quantity * SERVER_MESSAGES;
   pthread_t nodesThreads [nThreads];
   iThreads = 0;
   portno = 5101;

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
   serv_addr.sin_port = htons(portno);

   //Binding server socket
   if (bind(serverFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }

   //Listening for clientsStatuses
   if (listen(serverFd, 5) < 0) {
      perror("ERROR listening");
      exit(1);
   }

   while (1) {

      // Accept request connections
      if ((clientFd = accept(serverFd, (struct sockaddr *)NULL, NULL)) < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      printf("Client connected with socket %d.\n", clientFd);
      buffer = NULL;
      buffer = readSocket(clientFd, 2, 1, &bytesRead);

      //TODO: Considerar que contiene el buffer cuando se recibieron simultaneamente
      // multiples mensajes previo a la lectura
      printf("Message received, '%s' of length %d.\n", buffer, bytesRead);

      if (n < 0) {
        perror("ERROR reading message from slave node");
        exit(1);
      }
      if (strcmp(buffer, CONNECT_REQUEST_MESSAGE) == 0) {
          printf("Some node wrote us by a CONNECT_REQUEST_MESSAGE\n");
          printf("connected server: %d, total: %d\n", clientsStatuses.connected, clientsStatuses.quantity);

          threadErr = pthread_create(&nodesThreads[iThreads], NULL, &connectReqMessage, &clientFd);
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
            clientsStatuses.connected++;
            printf("inside lock: %d\n", clientsStatuses.connected);
            if (clientsStatuses.connected == clientsStatuses.quantity) {
              printf("inside if\n");
              pthread_cond_broadcast(&sendStart);
            }
          pthread_mutex_unlock(&lock);

      } else if (strcmp(buffer, REPORT_MESSAGE) == 0) {
          printf("Some node wrote us by a REPORT_MESSAGE\n");

          clientsStatuses.reported++;

          rp->sock = clientFd;
          printf("socket: %d, rp->sock: %d\n", clientFd, rp->sock);
          memcpy(rp->hostname, &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr));

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
      if (clientsStatuses.reported == clientsStatuses.quantity) break;
      //   TODO: Considerar que la cantidad de hilos en iThreads se corresponda a
      //        la cantidad de hilos que debieron haberse usado

   }
   for (n = nThreads; n >= 0; n--)
      pthread_join(nodesThreads[n], NULL);

   pthread_mutex_destroy(&lock);
   pthread_cond_destroy(&sendStart);
   return 0;
}

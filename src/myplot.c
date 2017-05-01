#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../include/benchmark.h"
#include "../include/mymessages.h"
#include "../include/myswitch.h"
#include "../include/myreport.h"
#include "../include/myplot.h"
#include "../include/gnuplot_i.h"

int plotLines(char *input, char *name){
  gnuplot_ctrl *h1;
  char numbertext[10];
  char namebuffer[MAX_NAME_LINES];
  char *xlabel = (char *) malloc(50 + 1);
  char *ylabel = (char *) malloc(50 + 1);

  int i = 0;
  int m = 0;
  int tam = 0;
  int aux = 0;
  int gotLabelX = 0;
  int gotLabelY = 0;
  double number = 0;

  h1 = gnuplot_init();

  struct fakeSwResults *xvaluesP = (struct fakeSwResults *)malloc(sizeof xvaluesP + (sizeof number) * MAX_LENG + (sizeof namebuffer));
  struct fakeSwResults *checkpoint = xvaluesP;
  struct fakeSwResults *tmp;
  struct fakeSwResults *iterator;

  xvaluesP->next = NULL;
  //Inicializacion de Labels
  do {
   if (input[aux] != ',') {
     if(gotLabelX) {
       ylabel[i] = input[aux];
       i++;
     } else {
       xlabel[aux] = input[aux];
     }
   } else {
     if(!gotLabelX) {
       gotLabelX = 1;
     } else if (!gotLabelY) {
       gotLabelY = 1;
     }
   }
   aux++;
  }while (!(gotLabelX && gotLabelY));
  i = 0;

  printf("xLabel '%s' | ylabel '%s'\n", xlabel, ylabel);

  //Configuracion de nombres en graficas.
  gnuplot_set_ylabel(h1, ylabel);
  gnuplot_set_xlabel(h1, xlabel);
  //Reseteo de grafica
  gnuplot_resetplot(h1);
  //Configuracion de estilo
  gnuplot_setstyle(h1,"lines");
  //Configuracion de salida PNG
  gnuplot_cmd(h1, "set grid ytics lc rgb \"#bbbbbb\" lw 1 lt 0");
  gnuplot_cmd(h1, "set grid xtics lc rgb \"#bbbbbb\" lw 1 lt 0");
  gnuplot_cmd(h1, "set key rmargin");
  gnuplot_cmd(h1, "set terminal pngcairo size 1024, 768");


  //Inicializacion de nombres
  do {
    if (input[aux] == ',') {
      tmp = (struct fakeSwResults *)malloc(sizeof(struct fakeSwResults) + (sizeof number) * MAX_LENG + (sizeof namebuffer));
      tmp->next = NULL;
      xvaluesP->next = tmp;
      printf("name %s\n", xvaluesP->name);
      xvaluesP = tmp;
      tam = 0;
    } else {
      xvaluesP->name[tam] = input[aux];
      tam++;
    }
    aux++;
  }while (input[aux] != ';');
  aux++;
  printf("name %s\n", xvaluesP->name);
  xvaluesP = checkpoint;

  printf("Values %c\n", input[aux]);
  //Graficacion
  while (input[aux] != ';'){
    //Texto representativo de un number de  10 digitos
    for(m = 0; m < 10; m++){
      numbertext[m] = '\0';
    }
    tam = 0;
    //Construye number
    while ((input[aux] != ',') && ( input[aux] != ';')){
      numbertext[tam] = input[aux];
      aux++;
      tam++;
    }
    number = atoi(numbertext);

    xvaluesP->x[i] = number;
    xvaluesP = xvaluesP->next;

    if (input[aux] == ';') {
      i++;
      xvaluesP = checkpoint;
    }
    aux++;
  }
  printf("graphication \n");
  //Graficacion
  iterator = checkpoint->next;
  char * command = (char*)malloc(150);
  snprintf(command, 150, "set output \"../reports/charts/%s.png\"", name);
  while (iterator != NULL){
    gnuplot_cmd(h1, command);
    gnuplot_plot_xy(h1, checkpoint->x, iterator->x, i, iterator->name);
    iterator = iterator->next;
  }
  return 0;
}

int plotFinalResults (char *input) {
  gnuplot_ctrl *h1;
  char names [MAX_VALUES][MAX_NAME_GENERAL];
  double xvalues [MAX_SW][MAX_VALUES];

  int i = 0;
  int j = 0;
  int z = 0;
  int w = 0;
  int m = 0;
  int tam = 0;
  int aux = 0;
  double number = 0;
  char numbertext[20];

  double x[10000];
  double y[10000];

  h1 = gnuplot_init();

  //Configuracion de nombres en graficas.
  gnuplot_set_ylabel(h1, "flows/sec") ;
  gnuplot_set_xlabel(h1, "Milliseconds") ;

  //Configuracion de salida PNG
  gnuplot_cmd(h1, "set terminal png medium size 1024,768");

  //Inicializacion de nombres
  for (i = 0; i < MAX_VALUES; i++) {
    while((input[aux] != ',') && ( input[aux] != ';')){
      names[i][tam] = input[aux];
      tam = tam + 1;
      aux = aux + 1;
    }
    printf("%s\n", names[i]);
    aux = aux + 1;
    tam = 0;
  }

  i = 0;
  j = 0;

  while (input[aux] != ';'){
    //Texto representativo de un number de  10 digitos
    for(m = 0; m < 20; m++){
      numbertext[m] = '\0';
    }
    tam = 0;
    while ((input[aux] != ',') && (input[aux] != ';')){
      numbertext[tam] = input[aux];
      aux = aux + 1;
      tam = tam +1;
    }

    number = atof(numbertext);
    xvalues[i][j] = number;
    printf("%f <-- %s [%d,%d]\n",xvalues[i][j], numbertext,i,j);
    j = j + 1;
    if (input[aux] == ';'){
      i = i + 1;
      j = 0;
    }
    aux = aux + 1;
  }

  for (z = 0; z < i; z++) {
    for (j = 0; j < NPOINTS_GENERAL; j++) {
      y[j + (z * NPOINTS_GENERAL)] = (z + 1) + (j - 50) * 0.005;
    }
  }

  gnuplot_setstyle(h1, "impulses");
  for (w = 0; w < 2; w++) {
    for (z = 0; z < i; z++) {
      for (j = 0; j < NPOINTS_GENERAL; j++) {
        x[j + (z * NPOINTS_GENERAL)] = xvalues[z][w];
      }
    }
    gnuplot_cmd(h1, "set output \"resumen.png\"");
    gnuplot_plot_xy(h1, y, x, i * NPOINTS_GENERAL, "");
  }

  for (z = 0; z < i; z++) {
    y[z] = z + 1;
    x[z] = xvalues[z][2];
  }
  gnuplot_setstyle(h1, "points");
  gnuplot_cmd(h1, "set output \"resumen.png\"");
  gnuplot_plot_xy(h1, y, x, i, "");

  for (z = 0; z < i; z++) {
    y[z] = z + 1;
    x[z] = xvalues[z][3];
  }
  gnuplot_setstyle(h1, "lines");
  gnuplot_cmd(h1, "set output \"resumen.png\"");
  gnuplot_plot_xy(h1, y, x, i, "");

  return 0;
}

char * buildHeader(int type, int n, int mode) {
  char* header;
  char* checkpoint;

  int written;
  int index = 0;

  switch (type) {

    case VALUES:
      header = (char *)malloc(30 * (n + 2));
      written = snprintf(header, 40, "timestamp (sec),flows/sec,time");
      checkpoint = header;
      header += written;
      do {
        written = snprintf(header, 20, ",Virtual Switch %d", index);
        header += written;
        index++;
      }while(index < n);
      break;

    case AVGS:
      header = (char *)malloc(30 * (n + 2));
      written = snprintf(header, 40, "timestamp (ms),flows/sec,time");
      checkpoint = header;
      header += written;
      do {
        written = snprintf(header, 20, ",%s", reports[index].hostname);
        header += written;
        index++;
      }while(index < n);
      break;

    case RESULTS:
      header = (char *)malloc(50);
      if (mode == MODE_LATENCY) {
        written = snprintf(header, 50, "nodes,ms/response,node,MAX,MIN,AVG,STD DEV");
      } else {
        written = snprintf(header, 50, "nodes,response/sec,node,MAX,MIN,AVG,STD DEV");
      }
      checkpoint = header;
      header += written;
      break;

    case SNMP:
      //TODO: DEFINE OIDS HEADERS
      break;
  }

  snprintf(header, 2, "%c", CSV_NEWLINE);
  header = checkpoint;

  return header;
}

char * buildGraph(int clientFd, int id, int type, int flows, int rows, int mode){
  char *header = NULL;
  char *buffer = NULL;
  char *body = (char *)malloc(150 * rows);
  char *graph;

  int bodySize = 0;
  int addHeader = type == VALUES || type == SNMP || myreport->queues[type].first == NULL;

  buffer = readSocketLimiter(clientFd, 150 * rows, &bodySize);

  if (type == RESULTS) {
    bodySize = snprintf(body, bodySize + 20, "%s,%s", reports[id].hostname, buffer);
  } else {
    strcat(body, buffer);
  }

  if (addHeader) {
    header = buildHeader(type, flows, mode);
    graph = (char *)malloc(strlen(header) + bodySize + 1);
    strcpy(graph, header);
    strcat(graph, body);
  } else {
    graph = (char *)malloc(bodySize);
    strcpy(graph, body);
  }

  if(bodySize == 0) {
    perror("buildGraph()");
    exit(1);
  }

  printf("[GRAPH %d]%s[GRAPH %d]\n", type, graph, type);

  return graph;
}

int plotManagement(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange)
{
  int index;

  report *generalReport = (struct report*)malloc(sizeof(struct report));

  char *graphName = (char *)malloc(50);
  char *graph;

  for (index = 0; index < MAX_QUEUE; index++) {
    generalReport->queues[index].last = (struct message *)malloc(sizeof(struct message));
    generalReport->queues[index].first = (struct message *)malloc(sizeof(struct message));
  }

  if (myreport == NULL || reports == NULL) {
    perror("We can't graph snmpReport or reports");
    exit(1);
  }

  //LOOP ONLY WITH TEST_RANGE FLAG
  index = 0;
  do {
    graph = buildGraph(clientFd, id, VALUES, nSwitches, nLines, mode);
    enqueueMessage(graph, generalReport, VALUES, !DELIMIT, 150 * nLines);

    graph = buildGraph(clientFd, id, AVGS, clientsStatuses.quantity, nLines, mode);
    pthread_mutex_lock(&lock);
      enqueueMessage(graph, myreport, AVGS, !DELIMIT, 150 * nLines);
    pthread_mutex_unlock(&lock);

    graph = buildGraph(clientFd, id, RESULTS, 4, clientsStatuses.quantity, mode);
    pthread_mutex_lock(&lock);
      enqueueMessage(graph, myreport, RESULTS, !DELIMIT, 150);
    pthread_mutex_unlock(&ulock);

    if (!testRange) break;
    index++;
  }while (index < nSwitches);

  snprintf(graphName, 50, "%s.values.png", reports[id].hostname);
  plotLines(generalReport->queues[VALUES].first->buffer, graphName);

  return -1;
}

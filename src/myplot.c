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

int parseLines(gnuplot_ctrl *h1, char *input, flow *flows, char *name){
  char * command = (char*)malloc(150);
  char numberText[10];

  flow *checkpoint = flows;
  flow *iterator;

  int i = 0;
  int j = 0;
  int offset = 0;
  double number = 0;
  printf("parsing \n");

  while (input[offset] != ';'){
    //Texto representativo de un number de  10 digitos
    for(j = 0; j < 10; j++){
      numberText[j] = '\0';
    }
    //Construye number
    j = 0;
    while ((input[offset] != ',') && ( input[offset] != ';')){
      numberText[j] = input[offset];
      offset++;
      j++;
    }
    number = atof(numberText);
    flows->x[i] = number;
    flows = flows->next;

    if (input[offset] == ';') {
      i++;
      flows = checkpoint;
    }
    offset++;
  }

  printf("graphication \n");
  iterator = checkpoint->next;

  snprintf(command, 150, "set output \"../reports/charts/%s.png\"", name);
  //TODO: move this to plotLines
  printf("checkpoint %s\n", checkpoint->name);
  while (iterator != NULL) {
    printf("iteration %s\n", iterator->name);
    gnuplot_cmd(h1, command);
    gnuplot_plot_xy(h1, checkpoint->x, iterator->x, i, iterator->name);
    iterator = iterator->next;
  }
  printf("end\n");
  //TODO: close h1

  return -1;
}


int plotLines(struct queue input, int type, char *name){
  gnuplot_ctrl *h1;

  int i = 0;
  int offset = 0;
  int gotLabelX = 0;
  int gotLabelY = 0;

  char *xlabel = (char *)calloc(sizeof(char),50 + 1);
  char *ylabel = (char *)calloc(sizeof(char),50 + 1);

  struct message *iterator;
  struct flow *flows = (struct flow *)malloc(sizeof(struct flow));
  struct flow *checkpoint = flows;
  struct flow *tmp;

  printf("\nPLOTTING:\n HEADER:\n  %s\n BODY:\n  %s\n NAME:\n  %s\n", input.first->buffer, (input.first->back)->buffer, name);
  h1 = gnuplot_init();

  //Inicializacion de Labels
  do {
   if (input.first->buffer[offset] != ',') {
     if(gotLabelX) {
       ylabel[i] = input.first->buffer[offset];
       i++;
     } else {
       xlabel[offset] = input.first->buffer[offset];
     }
   } else {
     if(!gotLabelX) {
       gotLabelX = 1;
     } else if (!gotLabelY) {
       gotLabelY = 1;
     }
   }
   offset++;
  }while (!(gotLabelX && gotLabelY));
  printf("\nxLabel '%s' | ylabel '%s'\n", xlabel, ylabel);

  i = 0;
  do {
    if (input.first->buffer[offset] == ',') {
      tmp = (struct flow *)malloc(sizeof(struct flow));
      tmp->next = NULL;
      flows->next = tmp;
      printf("flowName '%s'\n", flows->name);
      flows = tmp;
      i = 0;
    } else {
      flows->name[i] = input.first->buffer[offset];
      i++;
    }
    offset++;
  }while (input.first->buffer[offset] != ';');
  offset++;
  printf("flowName '%s'\n", flows->name);
  flows = checkpoint;

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

  i = 0;
  iterator = input.first->back;
  switch (type) {
    case VALUES:
      parseLines(h1, iterator->buffer, flows, name);
      break;
    case AVGS:
      tmp = (struct flow *)malloc(sizeof(struct flow));
      *tmp = *flows;
      *checkpoint = *flows;

      while (iterator != NULL)
      {
        tmp = tmp->next;
        *(checkpoint->next) = *(tmp);
        (checkpoint->next)->next = NULL;
        printf("data %s tmp %s\n", iterator->buffer, checkpoint->name);
        parseLines(h1, iterator->buffer, checkpoint, name);
        iterator = iterator->back;
        i++;
      }
      break;
    case SNMP:
      break;
  }
  //TODO close h1
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
      header = (char *)malloc(30 * (n + 2) + 1);
      written = snprintf(header, 40, "timestamp (sec),flows/sec,time");
      checkpoint = header;
      header += written;
      do {
        written = snprintf(header, 20 + 1, ",Virtual Switch %d", index);
        header += written;
        index++;
      }while(index < n);
      break;

    case AVGS:
      header = (char *)malloc(30 * (n + 2) + 1);
      written = snprintf(header, 40 + 1, "timestamp (ms),flows/sec,time");
      checkpoint = header;
      header += written;
      do {
        written = snprintf(header, 20 + 1, ",%s", reports[index].hostname);
        header += written;
        index++;
      }while(index < n);
      break;

    case RESULTS:
      header = (char *)malloc(50 + 1);
      if (mode == MODE_LATENCY) {
        written = snprintf(header, 50 + 1, "nodes,ms/response,node,MAX,MIN,AVG,STD DEV");
      } else {
        written = snprintf(header, 50 + 1, "nodes,response/sec,node,MAX,MIN,AVG,STD DEV");
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

void buildGraph(report *myreport, int clientFd, int id, int type, int flows, int rows, int mode) {
  char *header = NULL;
  char *buffer = NULL;
  char *body = (char *)malloc(150 * rows + 1);

  int bodySize = 0;
  int addHeader = myreport->queues[type].first == NULL;

  buffer = readSocketLimiter(clientFd, 150 * rows, &bodySize);

  if (type == RESULTS) {
    bodySize = snprintf(body, strlen(buffer) + 20 + 1, "%s,%s", reports[id].hostname, buffer);
  } else {
    strncpy(body, buffer, strlen(buffer) + 1);
  }

  if (addHeader) {
    header = buildHeader(type, flows, mode);
    enqueueMessage(header, myreport, type, !DELIMIT, strlen(header));
  }

  if(bodySize == 0) {
    perror("buildGraph()");
    exit(1);
  }

  printf("\n[BODY %d]%s[BODY %d]\n", type, body, type);
  enqueueMessage(body, myreport, type, !DELIMIT, strlen(body));
}

int plotManagement(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange) {
  int index;
  report *generalReport = (struct report*)malloc(sizeof(struct report));
  char *graphName = (char *)malloc(50 + 1);

  for (index = 0; index < MAX_QUEUE; index++) {
    generalReport->queues[index].last = (struct message *)malloc(sizeof(struct message));
    generalReport->queues[index].first = (struct message *)malloc(sizeof(struct message));
    generalReport->queues[index].last = NULL;
    generalReport->queues[index].first = NULL;
  }

  if (myreport == NULL || reports == NULL) {
    perror("We can't graph snmpReport or reports");
    exit(1);
  }

  //LOOP ONLY WITH TEST_RANGE FLAG
  index = 0;
  do {
    buildGraph(generalReport, clientFd, id, VALUES, nSwitches, nLines, mode);

    pthread_mutex_lock(&lock);
      buildGraph(myreport, clientFd, id, AVGS, clientsStatuses.quantity, nLines, mode);
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&lock);
      buildGraph(myreport, clientFd, id, RESULTS, 4, clientsStatuses.quantity, mode);
    pthread_mutex_unlock(&lock);

    if (!testRange) break;
    index++;
  }while (index < nSwitches);

  snprintf(graphName, 50 + 1, "%s.values", reports[id].hostname);
  plotLines(generalReport->queues[VALUES], VALUES, graphName);

  snprintf(graphName, 4 + 1, "avgs");
  plotLines(myreport->queues[AVGS], AVGS, graphName);

  return -1;
}

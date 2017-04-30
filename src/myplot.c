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
  char names [VALUES][MAX_NAME_GENERAL];
  double xvalues [MAX_SW][VALUES];

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
  for (i = 0; i < VALUES; i++) {
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

int plotManagement(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange)
{
  report *generalReport = (struct report*)malloc(sizeof(struct report));
  report *finalReport = (struct report*)malloc(sizeof(struct report));

  char *header = (char *)malloc( 20 * (nSwitches + 2));
  char *graphName = (char *)malloc(50);
  char *nodeGraph;
  char *resultGraph;

  char *checkpoint;

  int bytesRead = 0;
  int index = 0;
  int written = 0;

  if (snmpReport == NULL || reports == NULL) {
    perror("We can't graph snmpReport or reports");
    exit(1);
  }


  written = snprintf(header, 20, "ms,flows/sec,time");
  checkpoint = header;
  header += written;

  do {
    written = snprintf(header, 20, ",Virtual Switch %d", index);
    header += written;
    index++;
  }while(index < nSwitches);
  snprintf(header, 2, "%c", CSV_NEWLINE);
  header = checkpoint;

  nodeGraph = (char *)malloc(strlen(header) + (150 * (nLines) + 50) + 1);
  strcpy(nodeGraph, header); // copy string one into the result.

  //LOOP ONLY WITH TEST_RANGE FLAG
  index = 0;
  do {
    resultGraph = NULL;
    resultGraph = readSocketLimiter(clientFd, 150 * nLines, &bytesRead);
    strcat(nodeGraph, resultGraph); // append string two to the result.
    printf("[GRAPH %d]\n%s\n[/GRAPH %d]\n",index, nodeGraph, index);
    enqueueMessage(nodeGraph, generalReport, !DELIMIT, 150 * nLines);

    resultGraph = NULL;
    resultGraph = readSocketLimiter(clientFd, 150, &bytesRead);
    printf("[RESULT_GRAPH %d]\n%s\n[/RESULT_GRAPH %d]\n",index, resultGraph, index);
    enqueueMessage(resultGraph, finalReport, !DELIMIT, 150);

    if (!testRange) break;
    index++;
  }while (index < nSwitches);

  snprintf(graphName, 50, "%s.Values.png", reports[id].hostname);
  plotLines(generalReport->queue->buffer, graphName);


  //TODO: Change result graph header depending of the mode
  if (mode == MODE_LATENCY) {
    //written = snprintf(header, 20, "ms,flows/sec");
  } else {
    //written = snprintf(header, 20, "ms,flows/sec");
  }

  if (bytesRead > 0) {;
    //reports[id].queue = generalReport->queue;
  }
  return bytesRead;
}

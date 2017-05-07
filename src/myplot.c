#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "../include/benchmark.h"
#include "../include/mymessages.h"
#include "../include/myswitch.h"
#include "../include/myreport.h"
#include "../include/mysnmp.h"
#include "../include/myplot.h"

int parseLines(gnuplot_ctrl *h1, char *input, flow *flows, char *name){
  char * command = (char*)calloc(450, sizeof(char));
  char numberText[10];

  flow *checkpoint = flows;
  flow *resultsIterator;

  int i = 0;
  int j = 0;
  int offset = 0;
  double number = 0;
  printf("PARSING\n");
  assert(name != NULL);

  while (input[offset] != ';'){
    //INITIALIZING TEXT NUMBER
    for(j = 0; j < 10; j++){
      numberText[j] = '\0';
    }
    //PARSING TEXT NUMBER
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

  printf("PLOTTING\n");
  resultsIterator = checkpoint->next;
  snprintf(command, 450, "set output \"../reports/charts/%s.png\"", name);
  while (resultsIterator != NULL) {
    gnuplot_cmd(h1, command);
    gnuplot_plot_xy(h1, checkpoint->x, resultsIterator->x, i, resultsIterator->name);
    resultsIterator = resultsIterator->next;
  }
  free(command);
  return -1;
}

char *parseResults(gnuplot_ctrl *h1, char *input, flow *flows, char *name, int n){
  double results [n][MAX_VALUES];

  int i = 0;
  int j = 0;
  int z = 0;
  int w = 0;
  int m = 0;
  int tam = 0;
  int offset = 0;
  double number = 0;
  char text[30];

  double x[10000];
  double y[10000];

  char * command = (char *) malloc(450);
  char * pointSize = (char *) malloc(20);

  snprintf(command, 450, "set output \"../reports/charts/%s.png\"", name);
  snprintf(pointSize, 20, "set pointsize 2");

  printf("PARSING\n");
  while (input[offset] != ';'){

    //XTIC
    while (input[offset] != ','){ offset++; }
    offset++;

    //LINE
    while (input[offset] != ';') {

      //NUMBER INITIALIZATION
      for(m = 0; m < 30; m++){
        text[m] = '\0';
      }
      tam = 0;

      //NUMBER
      while ((input[offset] != ',') && (input[offset] != ';')){
        text[tam] = input[offset];
        offset++;
        tam++;
      }

      number = atof(text);
      results[i][j] = number;
      j++;
      if (input[offset] != ';') {
        offset++;
      }
    }

    //COUNT
    j = 0;
    i++;
    offset++;
  }

  //FILL IMPULSES
  printf("PLOTTING BARS\n");
  for (z = 0; z < i; z++) {
    for (j = 0; j < NPOINTS_GENERAL; j++) {
      y[j + (z * NPOINTS_GENERAL)] = (z + 1) + (j - 50) * 0.0005;
    }
  }

  gnuplot_setstyle(h1, "impulses");
  gnuplot_cmd(h1, command);

  for (w = 0; w < 2; w++) {
    for (z = 0; z < i; z++) {
      for (j = 0; j < NPOINTS_GENERAL; j++) {
        x[j + (z * NPOINTS_GENERAL)] = results[z][w];
      }
    }
    flows = flows->next;
    gnuplot_plot_xy(h1, y, x, i * NPOINTS_GENERAL, flows->name);
  }

  printf("PLOTTING POINTS\n");
  for (z = 0; z < i; z++) {
    y[z] = z + 1;
    x[z] = results[z][2];
  }

  gnuplot_setstyle(h1, "points");
  gnuplot_cmd(h1, pointSize);
  gnuplot_cmd(h1, command);

  flows = flows->next;
  gnuplot_plot_xy(h1, y, x, i, flows->name);

  for (z = 0; z < i; z++) {
    y[z] = z + 1;
    x[z] = results[z][3];
  }

  printf("PLOTTING POINTS\n");
  gnuplot_setstyle(h1, "points");
  gnuplot_cmd(h1, command);

  flows = flows->next;
  gnuplot_plot_xy(h1, y, x, i, flows->name);

  free(command);
  return 0;
}

int setXtic(gnuplot_ctrl *h1, char *input){
  int i = 0;
  int written = 0;
  int offset = 0;
  int length = 0;
  char *nameYaxis = (char *)calloc(50 + 1, sizeof(char));
  char * xtics = (char *)malloc(50);
  char * checkpoint;
  char * buffer =(char *)malloc(strlen(input) + 3);
  snprintf(buffer, strlen(input) + 3, "%s", input);
  written = snprintf(xtics, 20, "set xtics (");
  checkpoint = xtics;
  xtics += written;

  written = 0;
  while(1){
    xtics += written;
    // GET IP
    i = 0;
    while (buffer[offset] != ','){
      nameYaxis[i] = '\0';
      nameYaxis[i] = buffer[offset];
      offset++;
      i++;
    }
    nameYaxis[i] = '\0';
    written = snprintf(xtics, 50, "\"%s\"", nameYaxis);
    xtics += written;

    while (buffer[offset] != ';'){
      offset++;
    }

    length++;
    offset++;

    if (buffer[offset] == ';') {
      written = snprintf(xtics, 10, " %d)", length);
      break;
    } else {
      written = snprintf(xtics, 10, " %d, ", length);
    }
  }
  xtics = checkpoint;

  gnuplot_cmd(h1, xtics);
  return length;
}

int plotGraph(struct queue input, int type, char *name){
  gnuplot_ctrl *h1;

  int i = 0;
  int written = 0;
  int offset = 0;
  int gotLabelX = 0;
  int gotLabelY = 0;

  char *buffer;
  char *xtics = (char *)malloc(450 * input.length + 1);
  char *xrange = (char *)malloc(450 + 1);
  char *xlabel = (char *)calloc(50 + 1, sizeof(char));
  char *ylabel = (char *)calloc(50 + 1, sizeof(char));

  struct message *resultsIterator = (struct message *)malloc(sizeof(struct message*));
  struct flow *flows = (struct flow *)malloc(sizeof(struct flow));
  struct flow *checkpoint = (struct flow *)malloc(sizeof(struct flow));
  struct flow *tmp;

  checkpoint = flows;

  printf("\nPLOTTING:\n HEADER:\n  %s\n BODY:\n  %s\n NAME:\n  %s\n", input.first->buffer, (input.first->back)->buffer, name);
  h1 = gnuplot_init();

  //Inicializacion de Labels
  printf("PARSING LABELS\n");
  do {
   if (input.first->buffer[offset] != ',') {
     if(gotLabelX) {
       ylabel[i] = '\0';
       ylabel[i] = input.first->buffer[offset];
       i++;
     } else {
       xlabel[offset] = '\0';
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
  printf("\nLABELS IN AXIS: X - '%s' | Y - '%s'\n", xlabel, ylabel);

  printf("PARSING LEGEND\n");
  i = 0;
  do {
    if (input.first->buffer[offset] == ',') {
      tmp = (struct flow *)malloc(sizeof(struct flow));
      tmp->next = NULL;
      flows->next = tmp;
      flows->name[i] = '\0';
      printf("FLOWNAME '%s'\n", flows->name);
      flows = tmp;
      i = 0;
    } else {
      flows->name[i] = '\0';
      flows->name[i] = input.first->buffer[offset];
      i++;
    }
    offset++;
  }while (input.first->buffer[offset] != ';');
  flows->name[i] = '\0';
  offset++;
  printf("FLOWNAME '%s'\n", flows->name);
  flows = checkpoint;

  //SETTING LABELS
  gnuplot_set_ylabel(h1, ylabel);
  gnuplot_set_xlabel(h1, xlabel);
  //RESETTING PLOTTER
  gnuplot_resetplot(h1);

  //GRAPH BASIC CONFIGURATION
  gnuplot_cmd(h1, "set grid xtics lc rgb \"#bbbbbb\" lw 1 lt 0");
  gnuplot_cmd(h1, "set grid ytics lc rgb \"#bbbbbb\" lw 1 lt 0");
  gnuplot_cmd(h1, "set key rmargin");
  gnuplot_cmd(h1, "set terminal pngcairo size 1024, 768");

  i = 0;
  resultsIterator = input.first->back;
  printf("START PLOT\n");
  if (type == VALUES || type == SNMP) {

    if (type == SNMP) {
      printf("SNMP GRAPH\n");
      buffer = (char *)malloc(50);
      snprintf(buffer, 50, "set yrange [0:]");
      gnuplot_cmd(h1, buffer);
    } else  {
      printf("VALUES GRAPH\n");
    }
    gnuplot_setstyle(h1,"lines");
    printf("START LINES\n");
    parseLines(h1, resultsIterator->buffer, flows, name);
    printf("END LINES\n");
    gnuplot_close(h1);

  } else if (type == AVGS) {
    printf("AVGS GRAPH\n");
    gnuplot_setstyle(h1,"lines");
    tmp = (struct flow *)malloc(sizeof(struct flow));
    *tmp = *flows;
    *checkpoint = *flows;
    while (resultsIterator != NULL)
    {
      tmp = tmp->next;
      checkpoint->next = (struct flow *)malloc(sizeof(struct flow));
      *(checkpoint->next) = *(tmp);
      (checkpoint->next)->next = NULL;
      printf("START LINE\n");
      parseLines(h1, resultsIterator->buffer, checkpoint, name);
      printf("END LINE\n");
      free(checkpoint->next);
      resultsIterator = resultsIterator->back;
    }
    gnuplot_close(h1);

  } else if (type == RESULTS) {
    printf("RESULTS GRAPH\n");

    //SET XTICS
    printf("SETTING XTIC\n");
    i = setXtic(h1, resultsIterator->buffer);

    //SET XRANGE
    printf("SETTING XRANGE\n");
    written = snprintf(xrange, 20, "set xrange [0:%d]", i + 1);
    if (written == 0) {
      perror("set xrange");
      exit(1);
    }
    gnuplot_cmd(h1, xrange);

    printf("START BARS, LINES AND POINTS\n");
    parseResults(h1, resultsIterator->buffer, flows, name , i);
    printf("END BARS, LINES AND POINTS\n");

    gnuplot_close(h1);

  }
  free(xtics);
  free(xrange);
  free(xlabel);
  free(ylabel);
  free(tmp);
  free(flows);
  printf("END PLOT\n");
  return 0;
}

char * buildHeader(int type, int n, int mode, int subMode) {
  char* header;
  char* checkpoint;
  const char* snmpLabelX = (subMode == 0) ? "%" : "KB";
  const char* snmpFlows = (subMode == 0) ? "Memory,CPU" : "Received,Emmitted";
  int written;
  int index = 0;

  printf("BUILDING HEADER\n");
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
      written = snprintf(header, 40 + 1, "timestamp (seg),flows/ms,time");
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
      header = (char *)malloc(50 + 1);
      written = snprintf(header, 50 + 1, "timestamp (sec),%s,time,%s", snmpLabelX, snmpFlows);
      checkpoint = header;
      header += written;
      break;
  }

  snprintf(header, 2, "%c", CSV_NEWLINE);
  header = checkpoint;
  return header;
}

//TODO: insertSNMP behaviour
void buildGraph(report *myreport, int clientFd, int id, int type, int flows, int rows, int mode) {
  char *header = NULL;
  char *buffer = NULL;
  char *body = (char *)malloc(450 * rows + 1);

  int bodySize = 0;
  int addHeader = myreport->queues[type].first == NULL;

  buffer = readSocketLimiter(clientFd, 450 * rows, &bodySize);

  if (type == RESULTS) {
    bodySize = snprintf(body, strlen(buffer) + 20 + 1, "%s,%s", reports[id].hostname, buffer);
  } else {
    strncpy(body, buffer, strlen(buffer) + 1);
    free(buffer);
  }

  if (addHeader) {
    header = buildHeader(type, flows, mode, 0);
    enqueueMessage(header, myreport, type, !DELIMIT, strlen(header));
  }

  if(bodySize == 0) {
    perror("buildGraph()");
    exit(1);
  }

  printf("\n[BODY %d]%s[BODY %d]\n", type, body, type);
  enqueueMessage(body, myreport, type, !DELIMIT, strlen(body));
}

int plotDistributed(){
  int x = 0;
  int written = 0;
  int first = 1;
  char *graphName = (char *)calloc(10 + 1, sizeof(char));
  char *newline = (char *)malloc(4 + 1);
  char *snmpHeader;
  mysnmp->queues[TIME].length = mysnmp->queues[TIME].length ? mysnmp->queues[TIME].length : 1;
  char *snmpBody = (char *)calloc(1 + 450 * mysnmp->queues[TIME].length, sizeof(char));
  char *checkpoint = snmpBody;
  struct report *tmp = (struct report *)malloc(sizeof(struct report) +  MAX_QUEUE * sizeof(struct message) * 2);
  struct message *resultsMessage = myreport->queues[RESULTS].first->back;
  struct message *resultsIterator = resultsMessage->back;
  struct message *timeIterator =  mysnmp->queues[TIME].first;
  struct message *firstIterator = mysnmp->queues[MEMORY].first;
  struct message *secondIterator = mysnmp->queues[CPU].first;

  //PLOTTING AVGS
  //GRAPH NAME
  printf("\nSETTING NAME GRAPH - AVG\n");
  snprintf(graphName, 8 + 1, "partials");
  plotGraph(myreport->queues[AVGS], AVGS, graphName);

  // PLOTTING RESULTS
  printf("\nCONCATENING RESULTS\n");
  myreport->queues[RESULTS].length = myreport->queues[RESULTS].length ? myreport->queues[RESULTS].length : 1;
  resultsMessage->buffer  = (char *)realloc(resultsMessage->buffer, myreport->queues[RESULTS].length * 450 + 2);
  if(newline  == NULL){
    free(newline);
    perror("realloc() while expanding results buffer");
    exit(1);
  }

  //CONCATENING NODES RESULTS
  while(resultsIterator != NULL){
    strncat(resultsMessage->buffer, resultsIterator->buffer, 450);
    free(resultsIterator->buffer);
    free(resultsIterator);
    resultsIterator = resultsIterator->back;
  }
  snprintf(newline, 2, "%c", CSV_NEWLINE);
  strncat(resultsMessage->buffer, newline, 2);

  //GRAPH NAME
  printf("SETTING NAME GRAPH - RESULT\n");
  snprintf(graphName, 8 + 1, "results");
  plotGraph(myreport->queues[RESULTS], RESULTS, graphName);

  // PLOTTING SNMP
  printf("\nSETTING NAME GRAPH - SNMP MEM&CPU\n");
  tmp->queues[SNMP].last = NULL;
  tmp->queues[SNMP].first = NULL;

  snmpHeader = buildHeader(SNMP, -1, -1, 0);
  enqueueMessage(snmpHeader, tmp, SNMP, !DELIMIT, 50);
  mysnmp->queues[TIME].length = mysnmp->queues[TIME].length ? mysnmp->queues[TIME].length : 1;

  printf("BUILDING BODY\n");
  for (x = 0; x < mysnmp->queues[TIME].length; x++) {
    written = snprintf(snmpBody, 450,"%s,%s,%s;", timeIterator->buffer, firstIterator->buffer, secondIterator->buffer);
    if (first) {
      checkpoint = snmpBody;
      first = 0;
    }
    snmpBody += written;
    timeIterator = timeIterator->back;
    firstIterator = firstIterator->back;
    secondIterator = secondIterator->back;
  }
  snprintf(snmpBody, 2, "%c", CSV_NEWLINE);
  snmpBody = checkpoint;
  enqueueMessage(snmpBody, tmp, SNMP, !DELIMIT, 450 * mysnmp->queues[TIME].length);

  snprintf(graphName, 9 + 1, "memAndCpu");
  plotGraph(tmp->queues[SNMP], SNMP, graphName);

  free(tmp->queues[SNMP].last);
  free(tmp->queues[SNMP].first);
  free(tmp);

  // PLOTTING SNMP
  printf("\nSETTING NAME GRAPH - SNMP NETWORK\n");
  snprintf(graphName, 8 + 1, "bandwith");

  x = 0;
  written = 0;
  first = 1;
  timeIterator =  mysnmp->queues[TIME].first;
  firstIterator = mysnmp->queues[IN].first;
  secondIterator = mysnmp->queues[OUT].first;
  snmpBody = (char *)calloc(1 + 450 * mysnmp->queues[TIME].length, sizeof(char));

  tmp = (struct report *)malloc(sizeof(struct report) +  MAX_QUEUE * sizeof(struct message) * 2);
  tmp->queues[SNMP].last = NULL;
  tmp->queues[SNMP].first = NULL;

  snmpHeader = buildHeader(SNMP, -1, -1, 1);
  printf("BUILDING BODY\n");
  enqueueMessage(snmpHeader, tmp, SNMP, !DELIMIT, 50);

  for (x = 0; x < mysnmp->queues[TIME].length; x++) {
    written = snprintf(snmpBody, 450,"%s,%s,%s;", timeIterator->buffer, firstIterator->buffer, secondIterator->buffer);
    if (first) {
      checkpoint = snmpBody;
      first = 0;
    }
    snmpBody += written;
    timeIterator = timeIterator->back;
    firstIterator = firstIterator->back;
    secondIterator = secondIterator->back;
  }
  snprintf(snmpBody, 2, "%c", CSV_NEWLINE);
  printf("BODY BUILT\n %s\n", snmpBody);
  snmpBody = checkpoint;
  enqueueMessage(snmpBody, tmp, SNMP, !DELIMIT, 450 * mysnmp->queues[TIME].length);

  plotGraph(tmp->queues[SNMP], SNMP, graphName);

  free(tmp->queues[SNMP].last);
  free(tmp->queues[SNMP].first);
  free(tmp);
  return 0;
}

//TODO: Implement NON DISTRIBUTED management
int plotNode(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange) {
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
  plotGraph(generalReport->queues[VALUES], VALUES, graphName);
  for (index = 0; index < MAX_QUEUE; index++) {
    free(generalReport->queues[index].last);
    free(generalReport->queues[index].first);
  }
  free(graphName);
  free(generalReport);
  printf("plotGraph ended\n");

  return 1;
}

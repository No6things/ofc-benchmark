#ifndef MYPLOT_H
#define MYPLOT_H
#include "../include/gnuplot_i.h"

#define SLEEP_LGTH  5
#define NPOINTS    150
#define NPOINTS_GENERAL 100
#define MAX_NAME_GENERAL 3
#define MAX_VALUES 4
#define MAX_LENG 100
#define MAX_NAME_LINES 20

typedef struct flow {
  char name[MAX_NAME_LINES];
  double x[MAX_LENG];
  struct flow *next;
} flow;

int parseLines(gnuplot_ctrl *h1, char *input, flow *flows, char *name);
char* parseResults(gnuplot_ctrl *h1, char *input, flow *flows, char *name, int id);
int plotGraph(struct queue input, int type, char *name);
int plotDistributed();
int plotNode(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange);

#endif

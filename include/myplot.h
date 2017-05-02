#ifndef MYPLOT_H
#define MYPLOT_H

#define SLEEP_LGTH  5
#define NPOINTS    150
#define NPOINTS_GENERAL 100
#define MAX_NAME_GENERAL 3
#define MAX_VALUES 4
#define MAX_SW 1000
#define MAX_LENG 1000
#define MAX_NAME_LINES 20

typedef struct flow {
  char name[MAX_NAME_LINES];
  double x[MAX_LENG];
  struct flow *next;
} flow;

int plotLines(struct queue input, int type, char *name);
int plotFinalResults(char *input);
int plotManagement(int clientFd, int id, int nSwitches, int nLines, int mode, int testRange);

#endif

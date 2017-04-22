#ifndef MYPLOT_H
#define MYPLOT_H

#define SLEEP_LGTH  5
#define NPOINTS    150
#define NPOINTS_GENERAL 100
#define MAX_NAME_GENERAL 3
#define VALUES 4
#define MAX_SW 1000
#define MAX_LENG 1000
#define MAX_NAME_LINES 20

typedef struct flow {
  int *values;
  int *times;
  int *resultPerTime;
  int N;
  int max;
  int min;
  int avg;
  int stdev;
}flow;

typedef struct fakeSwResults {
  char name[MAX_NAME_LINES];
  double x[MAX_LENG];
  struct fakeSwResults *next;
} fakeSwResults;

int plotLines (char *cadena);
int plotFinalResults(char *cadena, int swNum);
int plotManagement();

#endif

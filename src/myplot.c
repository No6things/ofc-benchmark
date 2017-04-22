#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/benchmark.h"
#include "../include/myreport.h"
#include "../include/myplot.h"
#include "../include/gnuplot_i.h"

int plotLines(char *cadena){
  gnuplot_ctrl *h1;
  char numerotext[10];
  char namebuffer[MAX_NAME];

  int i = 0;
  int m = 0;
  int tam = 0;
  int aux = 0;
  double numero = 0;

  h1 = gnuplot_init();

  struct fakeSwResults *xvaluesP = (struct fakeSwResults *)malloc(sizeof xvaluesP + (sizeof numero) * MAX_LENG + (sizeof namebuffer));
  struct fakeSwResults *checkpoint = xvaluesP;
  struct fakeSwResults *tmp;
  struct fakeSwResults *iterator;

  xvaluesP->next=NULL;

  //Configuracion de nombres en graficas.
  gnuplot_set_ylabel(h1, "flows/sec") ;
  gnuplot_set_xlabel(h1, "Milliseconds") ;
  //Reseteo de grafica
  gnuplot_resetplot(h1);
  //Configuracion de estilo
  gnuplot_setstyle(h1,"lines");
  //Configuracion de salida PNG
  gnuplot_cmd(h1, "set terminal png");

  //inicializacion de nombres
  do {
    if (cadena[aux] == ',') {
      tmp = (struct fakeSwResults *)malloc(sizeof(struct fakeSwResults) + (sizeof numero) * MAX_LENG + (sizeof namebuffer));
      tmp->next = NULL;
      xvaluesP->next = tmp;
      xvaluesP = tmp;
      aux = aux + 1;
      tam = 0;
    } else {
      xvaluesP->name[tam] = cadena[aux];
      tam = tam + 1;
      aux = aux + 1;
    }
  }while (cadena[aux] != ';');
  aux = aux + 1;
  xvaluesP = checkpoint;

  //Graficacion
  while (cadena[aux] != ';'){
    //Texto representativo de un numero de  10 digitos
    for(m = 0; m < 10; m++){
      numerotext[m] = '\0';
    }
    tam = 0;
    //Construye numero
    while ((cadena[aux] != ',') && ( cadena[aux] != ';')){
      numerotext[tam] = cadena[aux];
      aux = aux + 1;
      tam = tam + 1;
    }
    numero = atoi(numerotext);

    xvaluesP->x[i] = numero;
    xvaluesP = xvaluesP->next;

    if (cadena[aux] == ';') {
      i = i + 1;
      xvaluesP = checkpoint;
    }
    aux = aux + 1;
  }

  //Graficacion
  iterator = checkpoint->next;
  while (iterator != NULL){
    gnuplot_cmd(h1, "set output \"nodos.png\"");
    gnuplot_plot_xy(h1, checkpoint->x, iterator->x, i, iterator->name);
    iterator = iterator->next;
  }
  return 0;
}

int plotFinalResults (char *cadena, int swNum) {
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
  double numero = 0;
  char numerotext[20];

  double x[10000];
  double y[10000];

  h1 = gnuplot_init();

  //Configuracion de nombres en graficas.
  gnuplot_set_ylabel(h1, "flows/sec") ;
  gnuplot_set_xlabel(h1, "Milliseconds") ;

  //Configuracion de salida PNG
  gnuplot_cmd(h1, "set terminal png");

  //Inicializacion de nombres
  for (i = 0; i < VALUES; i++) {
    while((cadena[aux] != ',') && ( cadena[aux] != ';')){
      names[i][tam] = cadena[aux];
      tam = tam + 1;
      aux = aux + 1;
    }
    printf("%s\n", names[i]);
    aux = aux + 1;
    tam = 0;
  }

  i = 0;
  j = 0;

  while (cadena[aux] != ';'){
    //Texto representativo de un numero de  10 digitos
    for(m = 0; m < 20; m++){
      numerotext[m] = '\0';
    }
    tam = 0;
    while ((cadena[aux] != ',') && (cadena[aux] != ';')){
      numerotext[tam] = cadena[aux];
      aux = aux + 1;
      tam = tam +1;
    }

    numero = atoi(numerotext);
    xvalues[i][j] = numero;
    printf("%f <-- %s [%d,%d]\n",xvalues[i][j], numerotext,i,j);
    j = j + 1;
    if (cadena[aux] == ';'){
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

int plotManagement()
{
  report *aux = (report *)malloc(sizeof aux);
  char *line = (char *)malloc(150);
  int nLines = 0;
  struct inputValues *params = &benchmarkArgs;

  flow test[params->nNodes][params->nSwitches];

  if (snmpReport == NULL || reports == NULL) {
    return -1;
  } else {

  }
  return 0;
}

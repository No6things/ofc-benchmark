#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/myplot.h"
#include "../include/gnuplot_i.h"


int plot(FILE *file){

  gnuplot_ctrl *h1;

  //Initialize handler of one graphic
  h1 = gnuplot_init();

  double x2[NPOINTS];
  double x3[NPOINTS];
  char cadena[9999];
  char numerotext[10];
  int numero;

  int i = 0;
  int y = 0;
  int digito = 0;
  int tam = 0;
  int aux = 0;
  int ndato = 0;

  //Configuracion de nombres en graficas.
  gnuplot_set_ylabel(h1, "%") ;
  gnuplot_set_xlabel(h1, "Sec") ;
  //Reseteo de grafica
  gnuplot_resetplot(h1);
  //Configuracion de estilo
  gnuplot_setstyle(h1,"lines");

  //Llenado de Valores de puntos.
  aux = 0;
  i = 0;
  fgets(cadena, 9999, file);

  while (cadena[aux] != ';'){
    tam = 0;
    while ((cadena[aux] != ',') && (cadena[aux] != ';')) {
      numerotext[tam] = cadena[aux];
      aux = aux + 1;
      tam = tam + 1;
    }
    numero = 0;
    for (y = tam; y > 0; --y) {
      digito = (int)(numerotext[y -1] - 48);
      numero += digito * pow(10.0, tam - y);
    }

    switch (ndato) {
      case 1:
        x2[i] = numero;
        break;
      case 2:
        x3[i] = numero;
        break;
    }

    ndato = ndato + 1;
    aux = aux + 1;
    if (cadena[aux-1]==';') {
      i = i + 1;
      ndato = 0;
    }
  }

  gnuplot_cmd(h1, "set terminal png");
  gnuplot_cmd(h1, "set output \"memory-cpu.png\"");
  //Graficacion
  gnuplot_plot_x(h1, x2, NPOINTS, "Uso de CPU");
  gnuplot_cmd(h1, "set output \"memory-cpu.png\"");
  gnuplot_plot_x(h1, x3, NPOINTS, "Uso de Memoria");
  //Configuracion de salida PNG

  return -1;
}

int plotManagement()
{
  FILE *archivo;

	archivo = fopen ( "cpu-memoria.log", "r" );
  //TODO: Replace this for reports reading
  if (archivo == NULL)
  {
    printf("\nError de apertura del archivo. \n\n");
  }else{
    plot(archivo);
  }
  return -1;
}

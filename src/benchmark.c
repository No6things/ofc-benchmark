#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <getopt.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <netinet/tcp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../include/benchmark.h"

#define PROG_TITLE "USAGE: ofc [option]  # by Alberto Cavadia and Daniel Tovar 2016"

void myPrintHelloMake(void) {

  printf("Hello makefiles!\n");

  return;
}

int main(int argc, char * argv[])
{
  myPrintHelloMake();
  return(0);
}

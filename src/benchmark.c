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
#include "../include/myargs.h"
#include "../include/loci/loci.h"

#define PROG_TITLE "USAGE: ofcB [option]  # by Alberto Cavadia and Daniel Tovar 2016"

struct myargs my_options[] = { //Options for Tests
    {"controller",  'C', "Hostname of controller to connect to", STRING, {.string = "localhost"}},
    {"delay-inter-packets",    'd', "Delay inter-packet sending (in ms)", INTEGER, {.integer = 0}},
    {"delay",       'D', "Delay starting testing after features_reply is received (in ms)", INTEGER, {.integer = 0}},
    {"fields",      'f', "Packet fields modified", FLAG, {.flag = 0}},
    {"help",        'h', "Print this manual", NONE, {.none = 0}},
    {"learn-dst-macs",  'L', "Send gratuitious ARP replies to learn destination macs before testing", FLAG, {.flag = 1}},
    {"ms-per-test", 'm', "Test length in ms", INTEGER, {.integer = 1000}},
    {"pid-offset",  'O', "Switch DPID offset", INTEGER, {.integer = 1}},
    {"packets",     'p', "Number of packets", INTEGER, {.integer = 0}},
    {"port",        'P', "Controller port",  INTEGER, {.integer = OF_TCP_PORT}},
    {"ranged-test", 'r', "Test range of 1..$n packages", FLAG, {.flag = 0}},
    {"random",       'o', "Sending order is random", FLAG, {.flag = 0}},
    {"switches",    's', "Number of switches", INTEGER, {.integer = 16}},
    {"size",        'S', "Size of packets", INTEGER, {.integer = 0}}, //todo
    {"throughput",  't', "Test throughput instead of latency", NONE, {.none = 0}},
    {0, 0, 0, 0}
};

//RUN TEST

//TIMEOUT

//CONNECT

//MAIN

int main(int argc, char * argv[])
{
  myargsManual(my_options, PROG_TITLE, 1);
  //INITIALIZE

  //PARSE ARGS LOOP

  //STATUS MSGS

  //TEST
      //CONNECTION
      //INIT FAKE SWITCH
      //RUN

  //SHOW RESULT
  return(0);
}

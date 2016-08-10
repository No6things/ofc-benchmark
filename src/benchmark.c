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

struct args options[] = { //Options for Tests. In case of change, also change main
    {"controller",  'C', "Hostname of controller to connect to", STRING, {.string = "localhost"}},
    {"packet-delay",'d', "Interpacket gap (in ms)", INTEGER, {.integer = 0}},
    {"delay",       'D', "Delay starting testing after features_reply is received (in ms)", INTEGER, {.integer = 0}},
    {"fields",      'f', "Packet fields modified", FLAG, {.flag = 0}},
    {"help",        'h', "Print this manual", NONE, {.none = 0}},
    {"learn-dst-macs",'L', "Send gratuitious ARP replies to learn destination macs before testing", FLAG, {.flag = 1}},
    {"ms-per-test", 'm', "Test length in ms", INTEGER, {.integer = 1000}},
    {"dpid-offset",  'O', "Switch DPID offset", INTEGER, {.integer = 1}},
    {"packets",     'p', "Number of packets", INTEGER, {.integer = 0}},
    {"port",        'P', "Controller port",  INTEGER, {.integer = OF_TCP_PORT}},
    {"ranged-test", 'r', "Test range of 1..$n packages", FLAG, {.flag = 0}},
    {"random",      'o', "Sending order is random", FLAG, {.flag = 0}},
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
  //INITIALIZE
  int i=0,
      j=0;

  const struct option * longOpts = argsToLong(options);
  char * shortOpts = argsToShort(options);

  char *  controllerHostname =argsGetDefaultStr(options,"controller");
  int     packetDelay =       argsGetDefaultInt(options, "packet-delay"),
          delay =             argsGetDefaultInt(options, "delay"),
          fields =            argsGetDefaultFlag(options, "fields"),
          learnDstMacs =      argsGetDefaultFlag(options, "learn-dst-macs"),
          msTestLen =         argsGetDefaultInt(options, "ms-per-test"),
          dpidOffset =        argsGetDefaultInt(options, "dpid-offset"),
          nPackets=           argsGetDefaultInt(options, "packets"),
          controllerPort =    argsGetDefaultInt(options, "port"),
          testRange =         argsGetDefaultFlag(options, "ranged-test"),
          nSwitches =         argsGetDefaultInt(options, "switches"),
          packetSize =        argsGetDefaultInt(options, "size"),
          random =            argsGetDefaultFlag(options, "random"),
          mode = MODE_LATENCY;

  //PARSE ARGS LOOP
  // TODO: HANDLE MALICIOUS DATA
  while(1)
  {
     int index = 0,
         c = getopt_long(argc, argv, shortOpts, longOpts, &index);

     if (c == -1)
         break;
     switch (c)
     {
         case 'C' :
             controllerHostname = strdup(optarg);
             break;
         case 'd':
             packetDelay = atoi(optarg);
             break;
         case 'D':
             delay = atoi(optarg);
             break;
         case 'f':
             fields = 1;
             break;
         case 'h':
             argsManual(options, PROG_TITLE,  1);
             break;
         case 'L':
             if(optarg)
                 learnDstMacs = ( strcasecmp("true", optarg) == 0 || strcasecmp("on", optarg) == 0 || strcasecmp("1", optarg) == 0);
             else
                 learnDstMacs = 1;
             break;
         case 'm':
             msTestLen = atoi(optarg);
             break;
         case 'O':
             dpidOffset = atoi(optarg);
             break;
         case 'p' :
             nPackets = atoi(optarg);
             break;
         case 'P' :
             controllerPort = atoi(optarg);
             break;
         case 'r':
             testRange = 1;
             break;
         case 'o':
             random = 1;
             break;
         case 's':
             nSwitches = atoi(optarg);
             break;
         case 'S':
             packetSize = atoi(optarg);
             break;
         case 't':
             mode = MODE_THROUGHPUT;
             break;
         default:
             argsManual(options, PROG_TITLE, 1);
     }
  }

  //STATUS MSGS
  fprintf(stderr, "ofcB: OpenFLow Controller Benchmarking Tool\n"
                  "   running in mode %s\n"
                  "   connecting to controller at %s:%d \n"
                  "   faking%s %d switches offset %d : %d ms per test\n"
                  "   %s destination mac addresses before the test\n"
                  "   starting test with %d ms delay after features_reply\n",
                  mode == MODE_THROUGHPUT? "'throughput'": "'latency'",
                  controllerHostname,  controllerPort,
                  testRange ? " from 1 to": "",
                  nSwitches,
                  dpidOffset,
                  msTestLen,
                  learnDstMacs ? "learning" : "NOT learning",
                  delay);

    //TEST INIT

    //  fakeswitches = malloc(n_fakeswitches * sizeof(struct fakeswitch));
    //  assert(fakeswitches);
  /*
      double *results;
      double  min = DBL_MAX;
      double  max = 0.0;
      double  v;
      results = malloc(tests_per_loop * sizeof(double));

  for( i = 0; i < n_fakeswitches; i++)
      {
          //CONNECTION
          int sock;
          double sum = 0;
          if (connect_delay != 0 && i != 0 && (i % connect_group_size == 0)) {
              if(debug)
                  fprintf(stderr,"Delaying connection by %dms...", connect_delay*1000);
              usleep(connect_delay*1000);
          }
          sock = make_tcp_connection(controller_hostname, controller_port,3000, mode!=MODE_THROUGHPUT );
          if(sock < 0 )
          {
              fprintf(stderr, "make_nonblock_tcp_connection :: returned %d", sock);
              exit(1);
          }
          if(debug)
              fprintf(stderr,"Initializing switch %d ... ", i+1);
          fflush(stderr);
          fakeswitch_init(&fakeswitches[i],dpid_offset+i,sock,BUFLEN, debug, delay, mode, total_mac_addresses, learn_dst_macs);
          if(debug)
              fprintf(stderr," :: done.\n");
          fflush(stderr);
          if(count_bits(i+1) == 0)  // only test for 1,2,4,8,16 switches
              continue;
          if(!should_test_range && ((i+1) != n_fakeswitches)) // only if testing range or this is last
              continue;
          //RUN
          for( j = 0; j < tests_per_loop; j ++) {
              if ( j > 0 )
                  delay = 0;      // only delay on the first run
              v = 1000.0 * run_test(i+1, fakeswitches, mstestlen, delay);
              results[j] = v;
  			if(j<warmup || j >= tests_per_loop-cooldown)
  				continue;
              sum += v;
              if (v > max)
                max = v;
              if (v < min)
                min = v;
          }


      //SHOW RESULT
  		int counted_tests = (tests_per_loop - warmup - cooldown);
          // compute std dev
          double avg = sum / counted_tests;
          sum = 0.0;
          for (j = warmup; j < tests_per_loop-cooldown; ++j) {
            sum += pow(results[j] - avg, 2);
          }
          sum = sum / (double)(counted_tests);
          double std_dev = sqrt(sum);

          printf("RESULT: %d switches %d tests "
              "min/max/avg/stdev = %.2lf/%.2lf/%.2lf/%.2lf responses/s\n",
                  i+1,
                  counted_tests,
                  min, max, avg, std_dev);
      }
*/
  return(0);
}

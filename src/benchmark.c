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


#include "../include/openflow.h"

#include "../include/myargs.h"
#include "../include/benchmark.h"
#include "../include/switch.h"


#define PROG_TITLE "USAGE: ofcB [option]  # by Alberto Cavadia and Daniel Tovar 2016"

//DEFAULT VALUES

struct args options[] = { //Options for Tests. In case of change, also change main
    {"cooldown", 'c', "Loops to be disregarded at test end (cooldown)", INTEGER, {.integer = 0}},
    {"controller",  'C', "Hostname of controller to connect to", STRING, {.string = "localhost"}},
    {"packet-delay",'d', "Interpacket gap (in ms)", INTEGER, {.integer = 0}},
    {"delay",       'D', "Delay starting testing after features_reply is received (in ms)", INTEGER, {.integer = 0}},
    {"fields",      'f', "Packet fields modified", FLAG, {.flag = 0}},
    {"help",        'h', "Print this manual", NONE, {.none = 0}},
    {"connect-delay",  'i', "Delay between groups of switches connecting to the controller (in ms)", INTEGER, {.integer = 0}},
    {"connect-group-size", 'I', "Number of switches in a connection delay group", INTEGER, {.integer = 1}},
    {"loops", 'l', "Loops per test", INTEGER, {.integer = 16}},
    {"learn-dst-macs",'L', "Send gratuitious ARP replies to learn destination macs before testing", FLAG, {.flag = 1}},
    {"ms-per-test", 'm', "Test length in ms", INTEGER, {.integer = 1000}},
    {"mac-addresses", 'M', "Unique source MAC addresses per switch", INTEGER, {.integer = 100000}},
    {"dpid-offset",  'O', "Switch DPID offset", INTEGER, {.integer = 1}},
    {"packets",     'p', "Number of packets", INTEGER, {.integer = 0}},
    {"port",        'P', "Controller port",  INTEGER, {.integer = OFP_TCP_PORT}},
    {"ranged-test", 'r', "Test range of 1..$n packages", FLAG, {.flag = 0}},
    {"random",      'o', "Sending order is random", FLAG, {.flag = 0}},
    {"switches",    's', "Number of switches", INTEGER, {.integer = 16}},
    {"size",        'S', "Size of packets", INTEGER, {.integer = 0}}, //todo
    {"throughput",  't', "Test throughput instead of latency", NONE, {.none = 0}},
    {"warmup",  'w', "Loops to be disregarded on test start (warmup)", INTEGER, {.integer = 1}},
    {0, 0, 0, 0}
};

//RUN TEST

double runtTest(int nSwitches,struct fakeswitch *switches, int mstestlen, int delay)
{
    struct timeval now, then, diff;
    struct  pollfd  *pollfds;
    int i;
    double sum = 0;
    double passed;
    int count;

    int total_wait = mstestlen + delay;
    time_t tNow;
    struct tm *tmNow;
    pollfds = malloc(nSwitches * sizeof(*pollfds));
    assert(pollfds);
    gettimeofday(&then,NULL);
    while(1)
    {
        gettimeofday(&now, NULL);
        timersub(&now, &then, &diff);
        if( (1000* diff.tv_sec  + (float)diff.tv_usec/1000)> total_wait)
            break;
        for(i = 0; i< nSwitches; i++)
            switchSetPollfd(&switches[i], &pollfds[i]);

        poll(pollfds, nSwitches, 1000);      // block until something is ready or 100ms passes

        for(i = 0; i< nSwitches; i++){
            ofp13SwitchHandleIo(&switches[i], &pollfds[i]);
        }
    }
    tNow = now.tv_sec;
    tmNow = localtime(&tNow);
    printf("%02d:%02d:%02d.%03d Testing %-3d Switches: flows/sec:  ", tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, (int)(now.tv_usec/1000), nSwitches);
    usleep(100000); // sleep for 100 ms, to let packets queue
    for( i = 0 ; i < nSwitches; i++)
    {
        count = switchGetCount(&switches[i]);
        printf("%d  ", count);
        sum += count;
    }
    passed = 1000 * diff.tv_sec + (double)diff.tv_usec/1000;
    passed -= delay;        // don't count the time we intentionally delayed
    sum /= passed;  // is now per ms
    printf("total = %lf per ms \n", sum);
    free(pollfds);
    return sum;
}

//CONNECTION

int timeoutConnect(int fd, const char * hostname, int port, int mstimeout) {
	int ret = 0;
	int flags;
	fd_set fds;
	struct timeval tv;
	struct addrinfo *res=NULL;
	struct addrinfo hints;
	char sport[BUFLEN];
	int err;

	hints.ai_flags          = 0;
	hints.ai_family         = AF_INET;
	hints.ai_socktype       = SOCK_STREAM;
	hints.ai_protocol       = IPPROTO_TCP;
	hints.ai_addrlen        = 0;
	hints.ai_addr           = NULL;
	hints.ai_canonname      = NULL;
	hints.ai_next           = NULL;

	snprintf(sport,BUFLEN,"%d",port);

	err = getaddrinfo(hostname,sport,&hints,&res);
	if(err|| (res==NULL))
	{
		if(res)
			freeaddrinfo(res);
		return -1;
	}



	// set non blocking
	if((flags = fcntl(fd, F_GETFL)) < 0) {
		fprintf(stderr, "timeoutConnect: unable to get socket flags\n");
		freeaddrinfo(res);
		return -1;
	}
	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		fprintf(stderr, "timeoutConnect: unable to put the socket in non-blocking mode\n");
		freeaddrinfo(res);
		return -1;
	}
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if(mstimeout >= 0)
	{
		tv.tv_sec = mstimeout / 1000;
		tv.tv_usec = (mstimeout % 1000) * 1000;

		errno = 0;

		if(connect(fd, res->ai_addr, res->ai_addrlen) < 0)
		{
			if((errno != EWOULDBLOCK) && (errno != EINPROGRESS))
			{
				fprintf(stderr, "timeoutConnect: error connecting: %d\n", errno);
				freeaddrinfo(res);
				return -1;
			}
		}
		ret = select(fd+1, NULL, &fds, NULL, &tv);
	}
	freeaddrinfo(res);

	if(ret != 1)
	{
		if(ret == 0)
			return -1;
		else
			return ret;
	}
	return 0;
}


int makeTcpConnectionFromPort(const char * hostname, unsigned short port, unsigned short sport,
        int mstimeout, int nodelay)
{
    struct sockaddr_in local;
    int s;
    int err;
    int zero = 0;

    s = socket(AF_INET,SOCK_STREAM,0);
    if(s<0){
        perror("makeTcpConnection: socket");
        exit(1);  // bad socket
    }
    if(nodelay && (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &zero, sizeof(zero)) < 0))
    {
        perror("setsockopt");
        fprintf(stderr,"makeTcpConnection::Unable to disable Nagle's algorithm\n");
        exit(1);
    }
    local.sin_family=PF_INET;
    local.sin_addr.s_addr=INADDR_ANY;
    local.sin_port=htons(sport);

    err=bind(s,(struct sockaddr *)&local, sizeof(local));
    if(err)
    {
        perror("makeTcpConnectionFromPort::bind");
        return -4;
    }

    err = timeoutConnect(s,hostname,port, mstimeout);

    if(err)
    {
        perror("makeTcpConnection: connect");
        close(s);
        return err; // bad connection
    }
    return s;
}

int makeTcpConnection(const char * hostname, unsigned short port, int mstimeout, int nodelay)
{
    return makeTcpConnectionFromPort(hostname,port, INADDR_ANY, mstimeout, nodelay);
}

//UTILS

int countBits(int n)
{
    int count =0;
    int i;
    for(i=0; i< 32;i++)
        if( n & (1<<i))
            count ++;
    return count;
}


//MAIN

int main(int argc, char * argv[])
{
  //INITIALIZE
  int i=0,
      j=0;

  struct fakeswitch *switches;
  const struct option * longOpts = argsToLong(options);
  char * shortOpts = argsToShort(options);

  char *  controllerHostname =argsGetDefaultStr(options,"controller");
  int     cooldown =          argsGetDefaultInt(options, "cooldown"),
          packetDelay =       argsGetDefaultInt(options, "packet-delay"),
          delay =             argsGetDefaultInt(options, "delay"),
          fields =            argsGetDefaultFlag(options, "fields"),
          connectDelay =      argsGetDefaultInt(options, "connect-delay"),
          connectGroupSize =  argsGetDefaultInt(options, "connect-group-size"),
          loopsPerTest =      argsGetDefaultInt(options, "loops"),
          learnDstMacs =      argsGetDefaultFlag(options, "learn-dst-macs"),
          msTestLen =         argsGetDefaultInt(options, "ms-per-test"),
          nMacAddresses =     argsGetDefaultInt(options, "mac-addresses"),
          dpidOffset =        argsGetDefaultInt(options, "dpid-offset"),
          nPackets=           argsGetDefaultInt(options, "packets"),
          controllerPort =    argsGetDefaultInt(options, "port"),
          testRange =         argsGetDefaultFlag(options, "ranged-test"),
          random =            argsGetDefaultFlag(options, "random"),
          nSwitches =         argsGetDefaultInt(options, "switches"),
          packetSize =        argsGetDefaultInt(options, "size"),
          warmup =            argsGetDefaultInt(options, "warmup"),
          debug = 0,
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
         case 'c' :
             cooldown = atoi(optarg);
             break;
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
         case 'i' :
             connectDelay = atoi(optarg);
             break;
         case 'I' :
             connectGroupSize = atoi(optarg);
             break;
         case 'l' :
             loopsPerTest = atoi(optarg);
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
         case 'M' :
             nMacAddresses = atoi(optarg);
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
         case 'w' :
             warmup = atoi(optarg);
             break;
         default:
          argsManual(options, PROG_TITLE, 1);
     }
  }

  //STATUS MSGS
  fprintf(stderr, "ofcB: OpenFlow Controller Benchmarking Tool\n"
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
  switches = malloc(nSwitches * sizeof(struct fakeswitch));
  assert(switches);

  double *results;
  double  min = DBL_MAX;
  double  max = 0.0;
  double  v;
  results = malloc(loopsPerTest * sizeof(double));

  for( i = 0; i < nSwitches; i++)
      {
          //CONNECTION
          int sock;
          double sum = 0;
          if (connectDelay != 0 && i != 0 && (i % connectGroupSize == 0)) {
              if(debug)
                  fprintf(stderr,"Delaying connection by %dms...", connectDelay*1000);
              usleep(connectDelay*1000);
          }
          sock = makeTcpConnection(controllerHostname, controllerPort, 3000, mode!=MODE_THROUGHPUT );
          if(sock < 0 )
          {
              fprintf(stderr, "make_nonblock_tcp_connection :: returned %d", sock);
              exit(1);
          }
          if(debug)
              fprintf(stderr,"Initializing switch %d ... ", i+1);
          fflush(stderr);
          switchInit(&switches[i],dpidOffset+i,sock,BUFLEN, debug, delay, mode, nMacAddresses, learnDstMacs , OFP131_VERSION);
          if(debug)
              fprintf(stderr," :: done.\n");
          fflush(stderr);
          if(countBits(i+1) == 0)  // only test for 1,2,4,8,16 switches
              continue;
          if(!testRange && ((i+1) != nSwitches)) // only if testing range or this is last
              continue;
          //RUN
          for( j = 0; j < loopsPerTest; j ++) {
              if ( j > 0 ) delay = 0;      // only delay on the first run
              v = 1000.0 * runtTest(i+1, switches, msTestLen, delay);
              results[j] = v;
        			if(j<warmup || j >= loopsPerTest-cooldown)
        				continue;
                    sum += v;
                    if (v > max)
                      max = v;
                    if (v < min)
                      min = v;
            }


          //SHOW RESULTS
  		    int countedTests = (loopsPerTest - warmup - cooldown);
          // compute std dev
          double avg = sum / countedTests;
          sum = 0.0;
          for (j = warmup; j < loopsPerTest-cooldown; ++j) {
            sum += pow(results[j] - avg, 2);
          }
          sum = sum / (double)(countedTests);
          double std_dev = sqrt(sum);

          printf("RESULT: %d switches %d tests "
              "min/max/avg/stdev = %.2lf/%.2lf/%.2lf/%.2lf responses/s\n",
                  i+1,
                  countedTests,
                  min, max, avg, std_dev);
      }

  return(0);
}

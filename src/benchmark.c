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

#include "../include/mysnmp.h"
#include "../include/myreport.h"
#include "../include/mymessages.h"
#include "../include/myserver.h"
#include "../include/myclient.h"
#include "../include/benchmark.h"
#include "../include/myswitch.h"


#define PROG_TITLE "USAGE: ofcB [option]  # by Alberto Cavadia and Daniel Tovar 2016"

//BENCHMARKING || Prints interval messages
double runtTest (int nSwitches, struct fakeswitch *switches, int mstestlen, int delay, report *rp) {
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
    gettimeofday(&then, NULL);
    int size = 150;
    char* message = (char *)malloc(size);
    int written = 0;
    char* tmp;
    while (1) {
      gettimeofday(&now, NULL);
      timersub(&now, &then, &diff);
      if ((1000 * diff.tv_sec  + (float)diff.tv_usec / 1000) > total_wait) {
        break;
      }
      for (i = 0; i < nSwitches; i++) {
        switchSetPollfd(&switches[i], &pollfds[i]);
      }
      poll(pollfds, nSwitches, 1000);      // block until something is ready or 100ms passes
      for (i = 0; i < nSwitches; i++) {
        ofp13SwitchHandleIo(&switches[i], &pollfds[i]);
      }
    }
    tNow = now.tv_sec;

    written += snprintf(message, size, "%ld:%d", now.tv_usec, nSwitches);
    tmp = message; //  start checkpoint
    message += written;
    usleep(100000); // sleep for 100 ms, to let packets queue

    for (i = 0; i < nSwitches; i++) {
      count = switchGetCount(&switches[i]);
      written = snprintf(message, size, ":%d", count);
      message += written;
      sum += count;
    }
    passed = 1000 * diff.tv_sec + (double)diff.tv_usec / 1000;
    passed -= delay;        // don't count the time we intentionally delayed
    sum /= passed;  // is now per ms
    snprintf(message, size, ":%lf", sum);
    message = tmp;
    enqueueMessage(message, myreport);
    free(pollfds);
    return sum;
}

// FINAL RESULT || Returns final report
char * formatResult (unsigned int mode, unsigned int i, int countedTests, double min, double max,double avg, double std_dev){
  char *buffer;
  size_t size;
  //ms/response
  if (mode == MODE_LATENCY) {
    size = snprintf(NULL, 0, "l:%.2lf,%.2lf,%.2lf,%.2lf",
            1000/min, 1000/max, 1000/avg, 1000/std_dev);

    buffer = (char *)malloc(size + 1);
    snprintf(buffer, size + 1, "l:%.2lf,%.2lf,%.2lf,%.2lf",
            1000/min, 1000/max, 1000/avg, 1000/std_dev);
  //response/s
  } else {
    size = snprintf(NULL, 0, "t:%.2lf,%.2lf,%.2lf,%.2lf",
            min, max, avg, std_dev);
    snprintf(buffer, size + 1, "t:%.2lf,%.2lf,%.2lf,%.2lf",
            min, max, avg, std_dev);

  }
  return buffer;
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

	snprintf(sport, BUFLEN, "%d", port);

	err = getaddrinfo(hostname, sport, &hints, &res);
	if (err|| (res == NULL)) {
		if (res)
			freeaddrinfo(res);
		return -1;
	}

	// set non blocking
	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		fprintf(stderr, "timeoutConnect: unable to get socket flags\n");
		freeaddrinfo(res);
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		fprintf(stderr, "timeoutConnect: unable to put the socket in non-blocking mode\n");
		freeaddrinfo(res);
		return -1;
	}
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (mstimeout >= 0) {
		tv.tv_sec = mstimeout / 1000;
		tv.tv_usec = (mstimeout % 1000) * 1000;

		errno = 0;

		if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
			if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
				fprintf(stderr, "timeoutConnect: error connecting: %d\n", errno);
				freeaddrinfo(res);
				return -1;
			}
		}
		ret = select(fd + 1, NULL, &fds, NULL, &tv);
	}
	freeaddrinfo(res);

	if (ret != 1) {
		if (ret == 0)
			return -1;
		else
			return ret;
	}
	return 0;
}

int makeTcpConnectionFromPort(const char * hostname, unsigned short port, unsigned short sport,
        int mstimeout, int nodelay) {
    struct sockaddr_in local;
    int s;
    int err;
    int zero = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("makeTcpConnection: socket");
        exit(1);
    }

    if (nodelay && (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &zero, sizeof(zero)) < 0)) {
        perror("setsockopt");
        fprintf(stderr,"makeTcpConnection::Unable to disable Nagle's algorithm\n");
        exit(1);
    }

    local.sin_family = PF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(sport);

    err = bind(s, (struct sockaddr *)&local, sizeof(local));
    if (err) {
        perror("makeTcpConnectionFromPort::bind");
        return -4;
    }

    err = timeoutConnect(s,hostname,port, mstimeout);

    if (err) {
        perror("makeTcpConnection: connect");
        close(s);
        return err;
    }
    return s;
}

int makeTcpConnection(const char * hostname, unsigned short port, int mstimeout, int nodelay) {
    return makeTcpConnectionFromPort(hostname,port, INADDR_ANY, mstimeout, nodelay);
}

//UTILS

int countBits(int n) {
    int count =0;
    int i;
    for(i=0; i< 32;i++)
        if( n & (1<<i))
            count ++;
    return count;
}

void initializeBenchmarking(int argc, char * argv[]) {
  struct inputValues *params = &benchmarkArgs;
  const struct option * longOpts = argsToLong(options);
  char *  shortOpts =         argsToShort(options);

  params->controllerHostname =argsGetDefaultStr(options,"controller");
  params->nodeMasterHostname =argsGetDefaultStr(options,"node-master");
  params->cooldown =          argsGetDefaultInt(options, "cooldown");
  params->packetDelay =       argsGetDefaultInt(options, "packet-delay");
  params->delay =             argsGetDefaultInt(options, "delay");
  params->debug =             argsGetDefaultFlag(options, "debug");
  params->fields =            argsGetDefaultFlag(options, "fields");
  params->connectDelay =      argsGetDefaultInt(options, "connect-delay");
  params->connectGroupSize =  argsGetDefaultInt(options, "connect-group-size");
  params->loopsPerTest =      argsGetDefaultInt(options, "loops");
  params->learnDstMacs =      argsGetDefaultFlag(options, "learn-dst-macs");
  params->msTestLen =         argsGetDefaultInt(options, "ms-per-test");
  params->nMacAddresses =     argsGetDefaultInt(options, "mac-addresses");
  params->master =            1;
  params->nNodes =            argsGetDefaultInt(options, "nodes");
  params->dpidOffset =        argsGetDefaultInt(options, "dpid-offset");
  params->nPackets=           argsGetDefaultInt(options, "packets");
  params->controllerPort =    argsGetDefaultInt(options, "port");
  params->testRange =         argsGetDefaultFlag(options, "ranged-test");
  params->random =            argsGetDefaultFlag(options, "random");
  params->nSwitches =         argsGetDefaultInt(options, "switches");
  params->packetSize =        argsGetDefaultInt(options, "size");
  params->mode =              MODE_LATENCY;
  params->warmup =            argsGetDefaultInt(options, "warmup");

  // TODO: HANDLE MALICIOUS DATA
  while(1) {
     int index = 0,
         c = getopt_long(argc, argv, shortOpts, longOpts, &index);

     if (c == -1)
         break;
     switch (c) {
         case 'c' :
             params->cooldown = atoi(optarg);
             break;
         case 'C' :
             params->controllerHostname = strdup(optarg);
             snmpDestination = (char *)malloc(20);
             snprintf(snmpDestination, 20, "%s", params->controllerHostname);
             break;
         case 'd':
             params->packetDelay = atoi(optarg);
             break;
         case 'D':
             params->delay = atoi(optarg);
             break;
         case 'f':
             params->fields = 1;
             break;
         case 'h':
             argsManual(options, PROG_TITLE,  1);
             break;
         case 'i' :
             params->connectDelay = atoi(optarg);
             break;
         case 'I' :
             params->connectGroupSize = atoi(optarg);
             break;
         case 'l' :
             params->loopsPerTest = atoi(optarg);
             break;
         case 'L':
             if(optarg)
                 params->learnDstMacs = ( strcasecmp("true", optarg) == 0 || strcasecmp("on", optarg) == 0 || strcasecmp("1", optarg) == 0);
             else
                 params->learnDstMacs = 1;
             break;
         case 'm':
             params->msTestLen = atoi(optarg);
             break;
         case 'M' :
             params->nMacAddresses = atoi(optarg);
             break;
         case 'n':
             params->nodeMasterHostname = strdup(optarg);
             if (strcasecmp(params->nodeMasterHostname, "localhost")) params->master = 0;
             break;
         case 'N':
             params->nNodes = atoi(optarg);
             break;
         case 'O':
             params->dpidOffset = atoi(optarg);
             break;
         case 'p' :
             params->nPackets = atoi(optarg);
             break;
         case 'P' :
             params->controllerPort = atoi(optarg);
             break;
         case 'r':
             params->testRange = 1;
             break;
         case 'o':
             params->random = 1;
             break;
         case 's':
             params->nSwitches = atoi(optarg);
             break;
         case 'S':
             params->packetSize = atoi(optarg);
             break;
         case 't':
             params->mode = MODE_THROUGHPUT;
             break;
         case 'w' :
             params->warmup = atoi(optarg);
             break;
         case 'x' :
             params->debug = atoi(optarg);
             break;
         default:
          argsManual(options, PROG_TITLE, 1);
     }
  }

  //START APPLICATION MESSAGE
  fprintf(stderr, "ofcB: OpenFlow Controller Benchmarking Tool\n"
                  "   running in mode %s\n"
                  "   connecting to controller at %s:%d \n"
                  "   test distributed among %d nodes, beign %s the master\n"
                  "   faking %s %d switches offset %d : %d ms per test\n"
                  "   %s destination mac addresses before the test\n"
                  "   starting test with %d ms delay after features_reply\n",
                  params->mode == MODE_THROUGHPUT ? "'throughput'": "'latency'",
                  params->controllerHostname, params->controllerPort,
                  params->nNodes, params->nodeMasterHostname,
                  params->testRange ? "from 1 to": "", params->nSwitches, params->dpidOffset, params->msTestLen,
                  params->learnDstMacs ? "learning" : "NOT learning",
                  params->delay);
  printf("-------------------------------------------TEST-------------------------------------------\n" );
}

char * controllerBenchmarking() {
  struct fakeswitch *switches;
  struct inputValues *params = &benchmarkArgs;
  char *finalMessage;
  unsigned int i, j;
  int threadErr;
  pthread_t snmp_thread;

  myreport = (report *)malloc(sizeof(myreport));
  switches = (struct fakeswitch *)malloc(params->nSwitches * sizeof(struct fakeswitch));

  assert(switches);

  double *results;
  double  min = DBL_MAX;
  double  max = 0.0;
  double  v;
  results = malloc(params->loopsPerTest * sizeof(double));

  if (params->nNodes <= 1) {
    threadErr = pthread_create(&snmp_thread, NULL, &asynchronousSnmp, NULL);
    if (threadErr) {
      pthread_join(snmp_thread, NULL);
      perror("Creating SNMP thread");
      exit(1);
    }
  }
  for(i = 0; i < params->nSwitches; i++) {
    //CONNECTION
    int sock;
    double sum = 0;
    if (params->connectDelay != 0 && i != 0 && (i % params->connectGroupSize == 0)) {
        if (params->debug) {
            fprintf(stderr, "Delaying connection by %dms...", params->connectDelay * 1000);
        }
        usleep(params->connectDelay * 1000);
    }
    sock = makeTcpConnection(params->controllerHostname, params->controllerPort, 3000, params->mode != MODE_THROUGHPUT);

    if(sock < 0) {
        fprintf(stderr, "make_nonblock_tcp_connection :: returned %d", sock);
        exit(1);
    }
    if (params->debug) {
        fprintf(stderr,"Initializing switch %d ... ", i + 1);
    }

    //Initializing report
    myreport->sock = sock;
    myreport->list = NULL;

    fflush(stderr);
    switchInit(&switches[i], params->dpidOffset + i, sock, BUFLEN, params->debug, params->delay, params->mode, params->nMacAddresses, params->learnDstMacs, OFP131_VERSION);
    if (params->debug) {
        fprintf(stderr," :: done.\n");
    }
    fflush(stderr);
    if (countBits(i + 1) == 0)  // only test for 1,2,4,8,16 switches
        continue;
    if (!params->testRange && ((i + 1) != params->nSwitches)) // only if testing range or this is last
        continue;
    //RUN
    for(j = 0; j < params->loopsPerTest; j++) {
        if (j > 0) {
          params->delay = 0;      // only delay on the first run
        }
        v = 1000.0 * runtTest(i + 1, switches, params->msTestLen, params->delay, myreport);
        results[j] = v;
        if (j < params->warmup || j >= params->loopsPerTest - params->cooldown) {
          continue;
        }
        sum += v;
        if (v > max) {
          max = v;
        }
        if (v < min) {
          min = v;
        }
    }

    //SHOW RESULTS
    int countedTests = (params->loopsPerTest - params->warmup - params->cooldown);
    double avg = sum / countedTests;
    sum = 0.0;
    for (j = params->warmup; j < params->loopsPerTest - params->cooldown; ++j) {
      sum += pow(results[j] - avg, 2);
    }
    sum = sum / (double)(countedTests);
    double std_dev = sqrt(sum);

    finalMessage = (char*)malloc(150 * sizeof(char));
    finalMessage = formatResult(params->mode, i, countedTests, min, max, avg, std_dev);

    enqueueMessage(finalMessage, myreport);
    /*
    TODO: right now application cant handle many switches because the pointer used is the same defined as gloabl,
          offset it would make us lose the start of the report, is needed to create a checkpoint
          myreport++;
    */
  }
  if (params->nNodes <= 1) {
    snmpStop = 1;
    pthread_join(snmp_thread, NULL);
    displayMessages(snmpReport);
  }
  return (char *)" ";
}

//MAIN

int main(int argc, char * argv[]) {
  struct inputValues *params = &benchmarkArgs;

  initializeBenchmarking (argc, argv);

  if (params->nNodes > 1) {
    if (params->master) {
      printf("Im the master node\n");
      initializeSnmp();
      serverSide(params->nNodes);
    } else {
      printf("Im one of the slaves node\n");
      clientSide(params->nodeMasterHostname);
    }
  } else {
    printf("Im alone\n");
    initializeSnmp();
    controllerBenchmarking();
    /*
    TODO: Recibir arreglo de reportes de controllerBenchmarking()
    TODO: Imprimir reportes
    */
  }

  return 0;
}

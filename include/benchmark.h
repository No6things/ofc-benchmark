#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "../include/openflow.h"
#include "../include/myargs.h"

#ifndef BUFLEN
#define BUFLEN 65536
#endif

#define OFP131_VERSION 0x04
#define VERSION "1.0"

static struct args options[] = { //Options for Tests. In case of change, also change main
    {"cooldown", 'c', "Loops to be disregarded at test end (cooldown)", INTEGER, {.integer = 0}},
    {"controller",  'C', "Hostname of controller to connect to", STRING, {.string = "localhost"}},
    {"packet-delay",'d', "Interpacket gap (in ms)", INTEGER, {.integer = 0}},
    {"delay",       'D', "Delay starting testing after features_reply is received (in ms)", INTEGER, {.integer = 0}},
    {"debug",      'x', "Debug messages", FLAG, {.flag = 0}},
    {"fields",      'f', "Packet fields modified", FLAG, {.flag = 0}},
    {"help",        'h', "Print this manual", NONE, {.none = 0}},
    {"connect-delay",  'i', "Delay between groups of switches connecting to the controller (in ms)", INTEGER, {.integer = 0}},
    {"connect-group-size", 'I', "Number of switches in a connection delay group", INTEGER, {.integer = 1}},
    {"loops", 'l', "Loops per test", INTEGER, {.integer = 16}},
    {"learn-dst-macs",'L', "Send gratuitious ARP replies to learn destination macs before testing", FLAG, {.flag = 1}},
    {"ms-per-test", 'm', "Test length in ms", INTEGER, {.integer = 1000}},
    {"mac-addresses", 'M', "Unique source MAC addresses per switch", INTEGER, {.integer = 100000}},
    {"node-master", 'n', "Hostname of the node master to send results to", STRING, {.string = "localhost"}},
    {"nodes",      'N', "Number of nodes in distributed mode", INTEGER, {.integer = 1}},
    {"dpid-offset",  'O', "Switch DPID offset", INTEGER, {.integer = 1}},
    {"packets",     'p', "Number of packets", INTEGER, {.integer = 0}},
    {"port",        'P', "Controller port",  INTEGER, {.integer = OFP_TCP_PORT}},
    {"ranged-test", 'r', "Test range of 1..$n packages", FLAG, {.flag = 0}},
    {"random",      'R', "Sending order is random", FLAG, {.flag = 0}},
    {"switches",    's', "Number of switches", INTEGER, {.integer = 16}},
    {"size",        'S', "Size of packets", INTEGER, {.integer = 0}}, //todo
    {"throughput",  't', "Test throughput instead of latency", NONE, {.none = 0}},
    {"warmup",  'w', "Loops to be disregarded on test start (warmup)", INTEGER, {.integer = 1}},
    {0, 0, 0, 0}
};

static struct inputValues {
  const char *controllerHostname;
  const char *nodeMasterHostname;
  unsigned int cooldown;
  unsigned int packetDelay;
  unsigned int delay;
  unsigned int debug;
  unsigned int fields;
  unsigned int connectDelay;
  unsigned int connectGroupSize;
  unsigned int loopsPerTest;
  unsigned int learnDstMacs;
  unsigned int msTestLen;
  unsigned int nMacAddresses;
  unsigned int master;
  unsigned int nNodes;
  unsigned int dpidOffset;
  unsigned int nPackets;
  unsigned int controllerPort;
  unsigned int testRange;
  unsigned int random;
  unsigned int nSwitches;
  unsigned int packetSize;
  unsigned int mode;
  unsigned int warmup;
} benchmarkArgs;



char * controllerBenchmarking();
void initializeBenchmarking(int argc, char * argv[]);

#endif

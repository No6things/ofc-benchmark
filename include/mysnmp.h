#ifndef MYSNMP_H
#define MYSNMP_H
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define CPU 0
#define MEMORY 1
#define IN 2
#define OUT 3
#define MAX_OID 5

/*
* a list of hosts to query
*/
static  struct host {
   const char *name;
   const char *community;
  } hosts[] = {
   { "192.168.1.105",		"public" },
   { NULL }
 };

/*
 * a list of variables to query for
 */
static struct oid {
   const char *readableName;
   const char *name;
   oid Oid[MAX_OID_LEN];
   int oidLen;
 } oids[] = {
   {"RAM_SIZE",".1.3.6.1.4.1.2021.4.5.0"},
   {"RAM_USED",".1.3.6.1.4.1.2021.4.6.0"},     //Total RAM used
   {"CPU_IDLE",".1.3.6.1.4.1.2021.11.11.0"},   //Percentage idle of user CPU time
   {"BYTES_IN",".1.3.6.1.2.1.2.2.1.10.2"},
   {"BYTES_OUT", ".1.3.6.1.2.1.2.2.1.16.2"},
   { NULL }
};

static struct session {
  struct snmp_session *sess;		/* SNMP session data */
  struct oid *current_oid;		/* How far in our poll are we */
} sessions[sizeof(hosts)/sizeof(hosts[0])];

int snmpStop;
char * snmpDestination;
unsigned long ramSize;
unsigned long networkStartIn;
unsigned long networkStartOut;
int snmpStart;
struct timeval tStart;


void initializeSnmp (void);
void *asynchronousSnmp(void* context);

#endif

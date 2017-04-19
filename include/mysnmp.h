#ifndef MYSNMP_H
#define MYSNMP_H
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

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
   //{"CPU user",".1.3.6.1.4.1.2021.11.50"},
   //{"CPU nice",".1.3.6.1.4.1.2021.11.51"},
   //{"CPU system",".1.3.6.1.4.1.2021.11.52"},
   {"CPU_TIME",".1.3.6.1.4.1.2021.11.9.0"},   //Percentage of user CPU time
   {"BYTES_IN",".1.3.6.1.2.1.2.2.1.10.2"}, //
   {"BYTES_OUT", ".1.3.6.1.2.1.2.2.1.16.2"},
   {"ERRORS_IN",".1.3.6.1.2.1.2.2.1.14.2"}, //The number of inbound packets that could not be transmitted because of errors
   {"ERRORS_OUT",".1.3.6.1.2.1.2.2.1.20.2"}, //The number of outbound packets that could not be transmitted because of errors
   {"RAM_SIZE",".1.3.6.1.4.1.2021.4.5.0"},    //Total RAM in machine TODO: We could calculate percetage
   {"RAM_USED",".1.3.6.1.4.1.2021.4.6.0"},     //Total RAM used
   { NULL }
};
#define MAX_OID 5

static struct session {
  struct snmp_session *sess;		/* SNMP session data */
  struct oid *current_oid;		/* How far in our poll are we */
} sessions[sizeof(hosts)/sizeof(hosts[0])];

int snmpStop;
char * snmpDestination;

void initializeSnmp (void);
void *asynchronousSnmp(void* context);

#endif

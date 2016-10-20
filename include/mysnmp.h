#ifndef MYSNMP_H
#define MYSNMP_H
/*
 * a list of hosts to query

*/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

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
   {"15 minute Load",".1.3.6.1.4.1.2021.10.1.3.3"}, //15 minute Load
   {"Percentage of user CPU time",".1.3.6.1.4.1.2021.11.9.0"},   //Percentage of user CPU time
   {"Total Swap Size",".1.3.6.1.4.1.2021.4.3.0"},    //Total Swap Size
   {"Total RAM in machine",".1.3.6.1.4.1.2021.4.5.0"},    //Total RAM in machine
   {"Total RAM used",".1.3.6.1.4.1.2021.4.6.0"},     //Total RAM used
   { NULL }
};
#define MAX_OID 5

static struct session {
  struct snmp_session *sess;		/* SNMP session data */
  struct oid *current_oid;		/* How far in our poll are we */
} sessions[sizeof(hosts)/sizeof(hosts[0])];


void
initializeSnmp (void);

void
asynchronousSnmp(char* controller);

#endif

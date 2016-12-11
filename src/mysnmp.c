
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include "../include/mysnmp.h"

static struct oid *op;
void initializeSnmp (void)
{
  op=oids;
  /* Win32: init winsock */
  SOCK_STARTUP;  /* initialize library */
  init_snmp("asynchapp");

  /* parse the oids */
  while (op->name) {
    op->oidLen = sizeof(op->Oid)/sizeof(op->Oid[0]);
    if (!read_objid(op->name, op->Oid, &op->oidLen)) {
      snmp_perror("read_objid");
      exit(1);
    }
    op++;
  }
  op= op - sizeof(oids)/sizeof(oids[0])+1;
}

/*
 * simple printing of returned data
 */
static int printResult (int status, struct snmp_session *sp, struct snmp_pdu *pdu)
{
  char buf[1024];
  struct variable_list *vp;
  int i;
  struct timeval now;
  struct timezone tz;
  struct tm *tm;


  gettimeofday(&now, &tz);
  tm = localtime(&now.tv_sec);
  fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec,
          now.tv_usec);

  switch (status) {

  case STAT_SUCCESS:
    vp = pdu->variables;
    if (pdu->errstat == SNMP_ERR_NOERROR) {
      while (vp) {
        snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
        fprintf(stdout, "%s:%s\n",op->readableName, buf);
	       vp = vp->next_variable;
         op++;
      }
    }
    else {
      for (i = 1; vp && i != pdu->errindex; vp = vp->next_variable, i++){
        if (vp) snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
        else strcpy(buf, "(none)");
        fprintf(stdout, "%s: %s\n", sp->peername, buf, snmp_errstring(pdu->errstat));
      }
    }
    return 1;

  case STAT_TIMEOUT:
    fprintf(stdout, "Timeout\n");
    return 0;

  case STAT_ERROR:
    snmp_perror(sp->peername);
    return 0;
  }
  return 0;
}

int active_hosts;			/* hosts that we have not completed */

/*
 * response handler
 */
static int asynchResponse(int operation, struct snmp_session *sp, int reqid,
		    struct snmp_pdu *pdu, void *magic)
{
  struct session *host = (struct session *)magic;
  struct snmp_pdu *req;

  if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
    if (printResult(STAT_SUCCESS, host->sess, pdu)) {
      host->current_oid++;			/* send next GET (if any) */
      if (host->current_oid->name) {
      	req = snmp_pdu_create(SNMP_MSG_GET);
      	snmp_add_null_var(req, host->current_oid->Oid, host->current_oid->oidLen);
      	if (snmp_send(host->sess, req))
      	  return 1;
      	else {
      	  snmp_perror("snmp_send");
      	  snmp_free_pdu(req);
      	}
      }
    }
  }
  else
    printResult(STAT_TIMEOUT, host->sess, pdu);

  /* something went wrong (or end of variables)
   * this host not active any more
   */
  active_hosts--;
  return 1;
}

void asynchronousSnmp(char* controller)
{
  struct session *hs;
  struct host *hp;
  hosts->name=controller; //controller
  /* startup all hosts */
  hs = sessions;
  hp = hosts;
  struct snmp_pdu *req;
  struct snmp_session sess;
  snmp_sess_init(&sess);			/* initialize session */
  sess.version = SNMP_VERSION_2c;
  sess.peername = strdup(hp->name);
  sess.community = strdup(hp->community);
  sess.community_len = strlen(sess.community);
  sess.callback = asynchResponse;		/* default callback */
  sess.callback_magic = hs;

  printf("---SNMP---\n" );

  if (!(hs->sess = snmp_open(&sess))) {
    snmp_perror("snmp_open");
  }

  hs->current_oid = oids;
  req = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
  snmp_add_null_var(req, hs->current_oid->Oid, hs->current_oid->oidLen);
  if (snmp_send(hs->sess, req))
    active_hosts++;
  else {
    snmp_perror("snmp_send");
    snmp_free_pdu(req);
  }

  /* loop while any active hosts */
  while (active_hosts) {
    int fds = 0, block = 1;
    fd_set fdset;
    struct timeval timeout;

    FD_ZERO(&fdset);
    snmp_select_info(&fds, &fdset, &timeout, &block);
    fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
    if (fds < 0) {
        perror("select failed");
        exit(1);
    }
    if (fds){
        snmp_read(&fdset);
    }else{
        snmp_timeout();
    }
  }

  for (hp = hosts, hs = sessions; hp->name; hs++, hp++) {
    if (hs->sess) snmp_close(hs->sess);
  }
}

/***************************/
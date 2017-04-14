#ifndef MYREPORT_H
#define MYREPORT_H

static struct report {
  int sock;
  const char *hostname;
  const char *buffer;
 } reports[] = {
   { 0 , "", "" },
   { '\0' }
 };

#endif

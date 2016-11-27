#ifndef MYSNMP_H
#define MYSNMP_H

#include <pthread.h>

void *connectReqMessage (void *context);

void *connectAck (void *context);

void *startAckMessage (void *context);

void *reportMessage (void *context);


#endif

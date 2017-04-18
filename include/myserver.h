#ifndef MYSERVER_H
#define MYSERVER_H

#define SERVER_PORT 5101

void * serverSide(unsigned int nNodes);

void * clientManagement(void *context);

#endif

#ifndef MYDIST_H
#define MYDIST_H

#include <pthread.h>

#define PORT_DIST 5001

#define SERVER_MESSAGES 4
#define CONNECT_REQUEST "1"
#define CONNECT_ACK "3"
#define START_ACK "5"
#define REPORT "6"

#define CLIENT_MESSAGES 3
#define CONNECT_REPLY "2"
#define START "4"
#define REPORT_ACK "7"

#endif

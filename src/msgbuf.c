#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/msgbuf.h"

#ifndef MIN
#define MIN(x,y)  (((x) < (y))? (x) : (y))
#endif


struct msgbuf *  msgbufNew(int bufsize)
{
    struct msgbuf * mbuf;
    mbuf = (struct msgbuf *)malloc(sizeof(*mbuf));
    assert(mbuf);
    mbuf->len = bufsize;
    mbuf->buf = (char *)malloc(mbuf->len);
    assert(mbuf->len);
    mbuf->start = mbuf->end = 0;

    return mbuf;
}

/**********************************************************************/
int msgbufRead(struct msgbuf * mbuf, int sock)
{
    int count = read(sock, &mbuf->buf[mbuf->end], mbuf->len - mbuf->end);
    if(count>0)
        mbuf->end+=count;
    if( mbuf->end >= mbuf->len)     // resize buffer if need be
        msgbufGrow(mbuf);
    return count;
}
/**********************************************************************/
int msgbufReadAll(struct msgbuf * mbuf, int sock, int len)
{

    int count = 0;
    int tmp;
    while(count < len)
    {
        /** fprintf(stderr, "in msgbufReadAll... count = %d len = %d tmp = %d\n",
                count, len, tmp); **/
        tmp =msgbufRead(mbuf,sock);
        if((tmp == 0)  ||
                ((tmp<0) && (errno != EWOULDBLOCK ) && (errno != EINTR) && (errno != EAGAIN)))
            return tmp;
        if(tmp  > 0)
            count+=tmp;
    }
    return count;
}
/**********************************************************************/
int msgbufWrite(struct msgbuf * mbuf, int sock, int len)
{
	int send_len = mbuf->end - mbuf->start;
	if (len > 0)
	{
		if (send_len < len)
			return -1;
		if (send_len > len)
			send_len = len;
	}
    int count = write(sock, &mbuf->buf[mbuf->start], send_len);
    if(count>0)
        mbuf->start+=count;
    if(mbuf->start >= mbuf->end)
        mbuf->start = mbuf->end = 0;
    return count;
}
/**********************************************************************/
int msgbufWriteAll(struct msgbuf * mbuf, int sock, int len)
{
    int tmp=0,count=0;
    while(mbuf->start < mbuf->end)
    {
        tmp=msgbufWrite(mbuf, sock, len);
        if((tmp < 0) &&
                (errno != EAGAIN) &&
                (errno != EWOULDBLOCK) &&
                (errno != EINTR))

            return tmp;
        if(count > 0)
            count+=tmp;
    }
    return count;
}
/**********************************************************************/
void msgbufClear(struct msgbuf * mbuf)
{
    mbuf->start = mbuf->end = 0;
}
/**********************************************************************/
void msgbufGrow(struct msgbuf * mbuf)
{
    mbuf->len *=2 ;
    mbuf->buf = (char *)realloc(mbuf->buf, mbuf->len);
    if(mbuf->buf == NULL) {
      perror("msgbufGrow failed");
      printf("Buffer len: %d\n", mbuf->len);
    }
    assert(mbuf->buf);
}
/**********************************************************************/
void * msgbufPeek(struct msgbuf *mbuf)
{
   if(mbuf->start >= mbuf->end)
      return NULL;
  return (void *) &mbuf->buf[mbuf->start];
}
/**********************************************************************/
int msgbufPull(struct msgbuf *mbuf, char * buf, int count)
{
    int min = MIN(count, mbuf->end - mbuf->start);
    if( min <= 0)
        return -1;
    if(buf)     // don't write if NULL
        memcpy(buf, &mbuf->buf[mbuf->start], min);
    mbuf->start+=min;
    if(mbuf->start>= mbuf->end)
        mbuf->start = mbuf->end = 0;
    return min;
}
/**********************************************************************/
void msgbufPush(struct msgbuf *mbuf, char * buf, int count)
{
    while((mbuf->end + count) > mbuf->len)
        msgbufGrow(mbuf);
    memcpy(&mbuf->buf[mbuf->end], buf, count);
    mbuf->end += count;
}

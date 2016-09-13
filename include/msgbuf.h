#ifndef MSGBUF_H
#define MSGBUF_H

struct msgbuf
{
    char * buf;
    int len, start, end;
};


struct msgbuf *  msgbufNew(int bufsize);
int              msgbufRead(struct msgbuf * mbuf, int sock);
int              msgbufReadAll(struct msgbuf * mbuf, int sock, int len);
int              msgbufWrite(struct msgbuf * mbuf, int sock, int len);
int              msgbufWriteAll(struct msgbuf * mbuf, int sock, int len);
void             msgbufGrow(struct msgbuf *mbuf);
void             msgbufClear(struct msgbuf *mbuf);
void *           msgbufPeek(struct msgbuf *mbuf);
int              msgbufPull(struct msgbuf *mbuf, char * buf, int count);
void             msgbufPush(struct msgbuf *mbuf, char * buf, int count);

#define msgbufCountBuffered(mbuf) ((mbuf->end - mbuf->start))
#endif

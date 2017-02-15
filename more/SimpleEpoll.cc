#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/epoll.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1
#define AE_DELETED_EVENT_ID -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)
struct aeEvnetLoop;

typedef void
aeFileProc (struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

typedef struct aeFileEvent
{
  int mask;
  aeFileProc *rfileProc;
  aeFileProc *wfileProc;
  void *clientData;
} aeFileEvent;

typedef struct aeFiredEvent
{
  int fd;
  int mask;
} aeFiredEvent;

typedef struct aeEventLoop
{
  int maxfd;
  int setsize;
  aeFileEvent *events;
  aeFiredEvent *fired;
  void *apidata;
  int stop;
} aeEventLoop;

typedef struct aeApiState
{
  int epfd;
  struct epoll_event *events;
} aeApiState;

int
aeApiCreate (aeEventLoop *eventLoop)
{
  aeApiState *state = (aeApiState *) malloc (sizeof(aeApiState));

  if (!state)
    return -1;
  state->events = (struct epoll_event *) malloc (
      sizeof(struct epoll_event) * eventLoop->setsize);
  if (!state->events)
    {
      free (state);
      return -1;
    }
  state->epfd = epoll_create (1024);
  if (state->epfd == -1)
    {
      free (state->events);
      free (state);
      return -1;
    }
  eventLoop->apidata = state;
  return 0;
}
aeEventLoop *
aeCreateEventLoop (int setsize)
{

  aeEventLoop *eventLoop;
  int i;

  if ((eventLoop = (aeEventLoop *) malloc (sizeof(*eventLoop))) == NULL)
    goto err;
  eventLoop->events = (aeFileEvent *) malloc (sizeof(aeFileEvent) * setsize);
  eventLoop->fired = (aeFiredEvent *) malloc (sizeof(aeFiredEvent) * setsize);
  if (eventLoop->events == NULL || eventLoop->fired == NULL)
    goto err;
  eventLoop->setsize = setsize;
  eventLoop->maxfd = -1;
  if (aeApiCreate (eventLoop) == -1)
    goto err;

  for (i = 0; i < setsize; i++)
    eventLoop->events[i].mask = AE_NONE;
  return eventLoop;
  err: if (eventLoop)
    {
      free (eventLoop->events);
      free (eventLoop->fired);
      free (eventLoop);

    }
  return NULL;
}

int
aeApiAddEvent (aeEventLoop *eventLoop, int fd, int mask)
{
  aeApiState *state = (aeApiState *) eventLoop->apidata;
  struct epoll_event ee =
    { 0 };

  int op =
      eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  ee.events = 0;
  mask |= eventLoop->events[fd].mask;
  if (mask & AE_READABLE)
    ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE)
    ee.events |= EPOLLOUT;
  ee.data.fd = fd;
  if (epoll_ctl (state->epfd, op, fd, &ee) == -1)
    return -1;

  return 0;
}
int
aeCreateFileEvent (aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc,
		   void *clientData)
{
  if (fd >= eventLoop->setsize)
    {
      errno = ERANGE;
      printf ("This Socket is %d > setSize , Errno = %d", fd, errno);
      return AE_ERR;
    }
  aeFileEvent *fe = &eventLoop->events[fd];

  if (aeApiAddEvent (eventLoop, fd, mask) == -1)
    return AE_ERR;
  fe->mask |= mask;

  if (mask & AE_READABLE)
    fe->rfileProc = proc;
  if (mask & AE_WRITABLE)
    fe->wfileProc = proc;
  fe->clientData = clientData;
  if (fd > eventLoop->maxfd)
    eventLoop->maxfd = fd;
  return AE_OK;
}
void
aeApiDelEvent (aeEventLoop *eventLoop, int fd, int delmask)
{
  aeApiState *state = (aeApiState *) eventLoop->apidata;
  struct epoll_event ee =
    { 0 };
  int mask = eventLoop->events[fd].mask & (~delmask);

  ee.events = 0;
  if (mask & AE_READABLE)
    ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE)
    ee.events |= EPOLLOUT;
  ee.data.fd = fd;
  if (mask != AE_NONE)
    {
      epoll_ctl (state->epfd, EPOLL_CTL_MOD, fd, &ee);
    }
  else
    {
      epoll_ctl (state->epfd, EPOLL_CTL_DEL, fd, &ee);
    }
}
void
aeDeleteFileEvent (aeEventLoop *eventLoop, int fd, int mask)
{
  if (fd >= eventLoop->setsize)
    return;
  aeFileEvent *fe = &eventLoop->events[fd];
  if (fe->mask == AE_NONE)
    return;

  aeApiDelEvent (eventLoop, fd, mask);
  fe->mask = fe->mask & (~mask);
  if (fd == eventLoop->maxfd && fe->mask == AE_NONE)
    {
      int j;
      for (j = eventLoop->maxfd - 1; j >= 0; j--)
	{
	  if (eventLoop->events[j].mask != AE_NONE)
	    break;
	}
      eventLoop->maxfd = j;
    }
}

int
aeApiPoll (aeEventLoop *eventLoop, struct timeval *tvp)
{
  aeApiState *state = (aeApiState *) eventLoop->apidata;
  int retval, numevents = 0;

  retval = epoll_wait (state->epfd, state->events, eventLoop->setsize, -1);
  if (retval > 0)
    {
      int j;

      numevents = retval;
      for (j = 0; j < numevents; j++)
	{
	  int mask = 0;
	  struct epoll_event *e = state->events + j;

	  if (e->events & EPOLLIN)
	    mask |= AE_READABLE;
	  if (e->events & EPOLLOUT)
	    mask |= AE_WRITABLE;
	  if (e->events & EPOLLERR)
	    mask |= AE_WRITABLE;
	  if (e->events & EPOLLHUP)
	    mask |= AE_WRITABLE;

	  eventLoop->fired[j].fd = e->data.fd;
	  eventLoop->fired[j].mask = mask;
	}
    }
  return numevents;
}

int
aeProcessEvents (aeEventLoop *eventLoop, int flags)
{
  int processed = 0, numevents;

  numevents = aeApiPoll (eventLoop, NULL);
  for (int j = 0; j < numevents; j++)
    {
      aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];
      int mask = eventLoop->fired[j].mask;
      int fd = eventLoop->fired[j].fd;
      int rfired = 0;
      if (fe->mask & mask & AE_READABLE)
	{
	  rfired = 1;
	  fe->rfileProc (eventLoop, fd, fe->clientData, mask);

	}
      if (fe->mask & mask & AE_WRITABLE)
	{
	  if (!rfired || fe->wfileProc != fe->rfileProc)
	    fe->wfileProc (eventLoop, fd, fe->clientData, mask);
	}
      processed++;
    }
  return processed;
}

void
aeMain (aeEventLoop *eventLoop)
{
  eventLoop->stop = 0;
  while (!eventLoop->stop)
    {
      aeProcessEvents (eventLoop, AE_ALL_EVENTS);
    }
}


int SetReuseAddr(int fd) {
    int yes = 1;
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        return -1;
    }
    return 0;
}

int SetNoBlock(int fd){
  int flags;
  if((flags = fcntl(fd, F_GETFL)) == -1){
      return -1;
  }
  flags |= O_NONBLOCK;
  if(fcntl(fd, F_SETFL, flags) == -1){
      return -1;
  }
  return 0;
}
typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    int listener;

    listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if( listener == -1 )
        return -1;

    //允许多次绑定同一个地址。要用在socket和bind之间
 //           evutil_make_listen_socket_reuseable(listener);
    SetReuseAddr(listener);
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if( ::bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
        goto error;

    if( ::listen(listener, listen_num) < 0)
        goto error;


    //跨平台统一接口，将套接字设置为非阻塞状态
  //  evutil_make_socket_nonblocking(listener);
    SetNoBlock(listener);
    return listener;

    error:
        errno_save = errno;
        close(listener);
        errno = errno_save;

        return -1;
}

void accept_cb(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    int sockfd;

    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    sockfd = ::accept(fd, (struct sockaddr*)&client, &len );
    SetNoBlock(sockfd);

    printf("accept a client fd = %d\n", sockfd);


}

int
main (void)
{
  printf("hello world\n");
  int listener = tcp_server_init(9999, 10);
      if( listener == -1 )
      {
          perror(" tcp_server_init error ");
          return -1;
      }
      printf("listener fd = %d\n", listener);
  aeEventLoop *loop = aeCreateEventLoop (1024);
  aeCreateFileEvent(loop, listener, AE_READABLE, accept_cb, NULL);
  aeMain (loop);
  return 0;
}

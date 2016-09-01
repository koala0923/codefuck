#ifndef TCPSERVICE_H_
#define TCPSERVICE_H_

#include<string>

#include<event.h>
#include<listener.h>
#include<bufferevent.h>

#include "EventLoop.h"

class TcpService
{
public:
  TcpService (const unsigned short port);
  ~TcpService ();
  void
  Start ();

private:
  static void
  listener_cb (evconnlistener *listener, evutil_socket_t fd,
	       struct sockaddr *sock, int socklen, void *arg);

private:
  evconnlistener *m_pEvconnListener;
  unsigned short m_Port;
  std::string m_ServerIp;
};
#endif

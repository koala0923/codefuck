#include "TcpService.h"

#include<netinet/in.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include<stdio.h>

#include "TcpConnect.h"
#include "log4z.h"

TcpService::TcpService (const unsigned short port) :
    m_Port (port)
{
  m_pEvconnListener = NULL;
}

TcpService::~TcpService ()
{
  evconnlistener_free (m_pEvconnListener);
}

void
TcpService::Start (void)
{

  struct sockaddr_in sin;
  memset (&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (m_Port);
  event_base * base = EventLoop::Instance ()->GetEventBase ();
  m_pEvconnListener = evconnlistener_new_bind (
      base, TcpService::listener_cb, this,
      LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
      10, (struct sockaddr*) &sin, sizeof(struct sockaddr_in));
  if (m_pEvconnListener == NULL)
    {
      LOGE("Init EvconnListener error");
    }
  //LEV_OPT_LEAVE_SOCKETS_BLOCKING
}

void
TcpService::listener_cb (evconnlistener *listener, evutil_socket_t fd,
			 struct sockaddr *sock, int socklen, void *arg)
{
  struct sockaddr_in *client_addr = (struct sockaddr_in *) sock;
//  printf ("accept a client fd=%d, ip = %s, port = %d\n", fd,
//	  inet_ntoa (client_addr->sin_addr), client_addr->sin_port);
  LOGI(
      "accept a client fd=" << fd << ", ip =" << inet_ntoa (client_addr->sin_addr) << ", port = " << client_addr->sin_port);
  TcpConnect* connect = new TcpConnect (fd);
  if (connect != NULL)
    {
      connect->Init ();
      connect->SetAddr (sock);

    }

}

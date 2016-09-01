#include "UdpService.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>
#include <string.h>
#include <map>

#include "EventLoop.h"
#include "PduBase.h"
#include "StrUser.h"
#include "log4z.h"

typedef struct sockaddr_in clientAddress;
typedef std::map<std::string, clientAddress> ClientList;
ClientList clients;

#define BUF_SIZE                        4096

UdpService::UdpService (const unsigned short port) :
    m_Port (port)
{
  m_pEvent = NULL;
}

UdpService::~UdpService ()
{

}

int
UdpService::Start ()
{
  int sock_fd;
  int flag = 1;
  struct sockaddr_in sin;

  /* Create endpoint */
  if ((sock_fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      //perror ("socket()");
      LOGE("socket()");
      return -1;
    }

  /* Set socket option */
  if (setsockopt (sock_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
    {
      //perror ("setsockopt()");
      LOGE("setsockopt()");
      return 1;
    }

  /* Set IP, port */
  memset (&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl (INADDR_ANY);

  sin.sin_port = htons (m_Port);

  /* Bind */
  if (bind (sock_fd, (struct sockaddr *) &sin, sizeof(struct sockaddr)) < 0)
    {
      //perror ("bind() error\n");
      LOGE("bind() error");
      return -1;
    }
  else
    {
      //printf ("bind() success\n");
      LOGD("UDP Service bind() success");
    }

  event_base * base = EventLoop::Instance ()->GetEventBase ();
  if (base == NULL)
    {
      LOGE("Init event_base error");
      return -1;
    }
  struct event* m_pEvent = event_new (base, sock_fd, EV_READ | EV_PERSIST,
				      UdpService::read_cb, this);
  if (m_pEvent == NULL)
    {
      LOGE("Init event error");
      return -1;
    }

  event_add (m_pEvent, NULL);
  return 0;
}

void
UdpService::read_cb (int fd, short event, void *arg)
{
  char buf[BUF_SIZE];
  int len;
  unsigned int size = sizeof(struct sockaddr);
  struct sockaddr_in client_addr;

  memset (buf, 0, sizeof(buf));
  len = recvfrom (fd, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr,
		  &size);

  if (len == -1)
    {
      LOGE("recvfrom() error");
    }
  else
    {

      LOGD(
	  "Read len = "<< len << ", address is " << inet_ntoa (client_addr.sin_addr) << ", port is " << client_addr.sin_port << ", fd = " << fd);
      if (len > 1500)
	{

	  LOGE("packet length too big");
	  return;
	}
      unsigned int protoLen;
      memcpy (&protoLen, buf, 4);
      protoLen = ntohl (protoLen);
//      char *protobuf = new unsigned char[protoLen];
//      memset (protobuf, 0, protoLen);
//      memcpy (protobuf, buf, protoLen);
      LOGD(
      	  "protocolbuf length = " << protoLen);
      //assert(protoLen <= len - 4);
      StreamService::StreamExhange msg;
      CHECK_PB_PARSE_MSG(msg.ParseFromArray (buf + 4, protoLen));
      LOGD(
	  "receive Udp msg from user = " << msg.from_user_name() << " to " << msg.to_session_name_size()<< " users");

      CUser *pfromUser = CUserManager::Instance ()->GetUser (
	  msg.from_user_name ());
      if (pfromUser != NULL)
	{
	  pfromUser->SetUdpClientAddress (client_addr);
	}

      for (int i = 0; i < msg.to_session_name_size (); i++)
	{
	  std::string to_session_name = msg.to_session_name (i);
	  CUser *ptoSessionUser =  CUserManager::Instance ()->GetUser(to_session_name);
	   if(ptoSessionUser != NULL){

	       struct sockaddr_in Sessionaddr = ptoSessionUser->GetUdpClientAddress();
	       sendto (fd, buf, len, 0, (struct sockaddr *) &Sessionaddr, size);
	       LOGD(
	       	  "Udp Msg " << msg.from_user_name() << " >>>>>>>>>>>>>>>>> " << to_session_name );
	   }
	}
    }
}

//void
//UdpService::read_cb (int fd, short event, void *arg)
//{
//  char buf[BUF_SIZE];
//  int len;
//  unsigned int size = sizeof(struct sockaddr);
//  struct sockaddr_in client_addr;
//
//  memset (buf, 0, sizeof(buf));
//  len = recvfrom (fd, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr,
//		  &size);
//
//  if (len == -1)
//    {
//      LOGE("recvfrom() error");
//    }
//  else
//    {
//      printf ("Read: len [%d] - content: %s", len, buf);
//      printf ("address is %s , port is %d , fd = %d\n",
//	      inet_ntoa (client_addr.sin_addr), client_addr.sin_port, fd);
//
//      std::string receivemsg = buf;
//      std::string key = receivemsg.substr (0, 7);
//      clients[key] = client_addr;
//      if (key == "client2")
//	{
//
//	  ClientList::iterator ite;
//	  ite = clients.find ("client1");
//	  if (ite != clients.end ())
//	    {
//
//	      struct sockaddr_in clientaddr = clients["client1"];
//	      sendto (fd, buf, len, 0, (struct sockaddr *) &clientaddr, size);
//
//	    }
//	}
//      /* Echo */
//      //sendto (fd, buf, len, 0, (struct sockaddr *) &client_addr, size);
//    }
//}

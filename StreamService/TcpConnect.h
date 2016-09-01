#ifndef TCPCONNECT_H_
#define TCPCONNECT_H_
#include<string>

#include<bufferevent.h>
#include<event.h>
#include<map>

#include "PduBase.h"

class TcpConnect
{
public:
  TcpConnect (const int fd);
  ~TcpConnect ();
  void
  Init ();
  void
  SetAddr (const struct sockaddr* sockaddr);
  std::string
  GetAddr ();
  int
  Getfd ()
  {
    return m_fd;
  }
  ;
  void
  HandlePdu (CSsPdu* pPdu);
  int
  SendPdu (CSsPdu* pPdu);
  static void
  timeout_cb (int fd, short event, void *params);
  void
  OnTimer (unsigned long long cur_time);
  void
  Close ();
  void
  SetLastRecTick (unsigned long long cur_tick)
  {
    m_last_recv_tick = cur_tick;
  }
private:
  void
  _HandleHeartBeat (CSsPdu* pPdu);
  void
  _HandleLoginRequest (CSsPdu* pPdu);
  void
  _HandleCallRequest (CSsPdu* pPdu);
  void
  _HandleCallRespond (CSsPdu* pPdu);
  void
  _HandleHangUpRequest (CSsPdu* pPdu);
  bool
  _UserAuthentication (const std::string& username,
		       const std::string& password);
  static void
  socket_read_cb (bufferevent *bev, void *arg);
  static void
  socket_event_cb (bufferevent *bev, short events, void *arg);

private:
  int m_fd;

  bufferevent *m_pBuffEvent;

  unsigned short m_Port;
  std::string m_ClientIp;
  unsigned long long int m_last_send_tick;
  unsigned long long int m_last_recv_tick;
};

typedef std::map<int, TcpConnect*> ConnMap_t; //所有连接的map
//extern ConnMap_t g_conn_map;
#endif

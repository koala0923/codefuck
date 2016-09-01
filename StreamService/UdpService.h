#ifndef UDPSERVICE_H_
#define UDPSERVICE_H_
#include <string>

class UdpService
{
public:
  UdpService (const unsigned short port);
  ~UdpService ();
  int
  Start ();
private:
  static void
  read_cb (int fd, short event, void *arg);
private:
  struct event* m_pEvent;
  unsigned short m_Port;
  std::string m_ServerIp;
};
#endif

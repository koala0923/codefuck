#ifndef 	STRUSER_H_
#define 	STRUSER_H_

#include "TcpConnect.h"
#include <string>
#include <arpa/inet.h>
typedef struct sockaddr_in clientAddress;
class CUser
{
public:
  CUser (const std::string username);
  ~CUser ();
  void
  SetConnect (TcpConnect* connect)
  {
    m_tcp_connect = connect;
  }
  TcpConnect*
  GetConnect (void)
  {
    return m_tcp_connect;
  }
  std::string&
  GetUsername (void)
  {
    return m_user_name;
  }
  clientAddress GetUdpClientAddress(void)
  {
    return m_udpAddress;
  }

  void SetUdpClientAddress(clientAddress address)
  {
    m_udpAddress = address;
    LOGD("set address = " << inet_ntoa(m_udpAddress.sin_addr) << " port = " << m_udpAddress.sin_port );
  }

private:
  std::string m_user_name;
  TcpConnect* m_tcp_connect;

  clientAddress m_udpAddress;
};

typedef std::map<int /* fd */, CUser*> UserMapbyFd_t;
typedef std::map<std::string /* 用户名 */, CUser*> UserMapbyName_t;

class CUserManager
{
public:
  ~CUserManager ();
  static CUserManager*
  Instance ();
  void
  AddUser (std::string name, CUser* user);
  void
  AddUser (int fd, CUser* user);
  void
  RemoveUser (int fd);
  void
  RemoveUser (std::string name);
  CUser*
  GetUser (std::string name);
  CUser*
  GetUser (int fd);
protected:
  CUserManager ();
private:
  static CUserManager* m_pUserManager;
  UserMapbyFd_t m_user_map_by_fd;
  UserMapbyName_t m_user_map_by_name;

};

#endif

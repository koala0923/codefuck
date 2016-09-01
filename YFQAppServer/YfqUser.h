#ifndef 	YFQUSER_H_
#define 	YFQUSER_H_

#include<map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Mutex.h>

typedef boost::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;

class CUser
{
public:
  CUser (const std::string username);
  ~CUser ();
  void
  SetConnect (WeakTcpConnectionPtr connect)
  {
    m_tcp_connect = connect;
  }
  WeakTcpConnectionPtr
  GetConnect (void)
  {
    return m_tcp_connect;
  }
  std::string&
  GetUsername (void)
  {
    return m_user_name;
  }


private:
  std::string m_user_name;
  WeakTcpConnectionPtr m_tcp_connect;

};

typedef boost::shared_ptr<CUser> CUserPtr;

typedef std::map<muduo::net::TcpConnection * /* TcpConnection 对象原始地址 */, CUserPtr> UserMapbyAd_t;
typedef std::map<std::string /* 用户名 */, CUserPtr> UserMapbyName_t;

class CUserManager
{
public:
  ~CUserManager ();
  static CUserManager*
  Instance ();
  void
  AddUser (std::string name, CUserPtr user);
  void
  AddUser (muduo::net::TcpConnection *pTcpConnection, CUserPtr user);
  void
  RemoveUser (muduo::net::TcpConnection *pTcpConnection);
  void
  RemoveUser (std::string name);
  CUserPtr
  GetUser (std::string name);
  CUserPtr
  GetUser (muduo::net::TcpConnection * pTcpConnection);
protected:
  CUserManager ();
private:
  static CUserManager* m_pUserManager;
  UserMapbyAd_t m_user_map_by_TcpConnectionAddr;
  UserMapbyName_t m_user_map_by_name;
  muduo::MutexLock mutex_;
};

#endif

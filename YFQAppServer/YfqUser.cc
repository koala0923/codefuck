#include "YfqUser.h"

CUser::CUser (const std::string username) :
    m_user_name (username)
{

}

CUser::~CUser (void)
{

}

CUserManager* CUserManager::m_pUserManager = NULL;

CUserManager::CUserManager ()
{

}

CUserManager::~CUserManager ()
{

}

CUserManager*
CUserManager::Instance (void)
{
  if (m_pUserManager == NULL)
    {
      m_pUserManager = new CUserManager;
    }
  return m_pUserManager;
}

void
CUserManager::AddUser (std::string name, CUserPtr user)
{
  muduo::MutexLockGuard lock (mutex_);
  m_user_map_by_name[name] = user;

}

void
CUserManager::AddUser (muduo::net::TcpConnection * pTcpConnection, CUserPtr user)
{
  muduo::MutexLockGuard lock (mutex_);
  m_user_map_by_TcpConnectionAddr[pTcpConnection] = user;

}

CUserPtr
CUserManager::GetUser (std::string name)
{
  CUserPtr pUser;
  muduo::MutexLockGuard lock (mutex_);
  UserMapbyName_t::iterator it = m_user_map_by_name.find (name);
  if (it != m_user_map_by_name.end ())
    {
      pUser = it->second;
    }
  return pUser;
}
CUserPtr
CUserManager::GetUser (muduo::net::TcpConnection * pTcpConnection)
{
  CUserPtr pUser;
  muduo::MutexLockGuard lock (mutex_);
  UserMapbyAd_t::iterator it = m_user_map_by_TcpConnectionAddr.find(pTcpConnection);
  if (it != m_user_map_by_TcpConnectionAddr.end ())
    {
      pUser = it->second;
    }
  return pUser;
}

void
CUserManager::RemoveUser (std::string name)
{
  muduo::MutexLockGuard lock (mutex_);
  m_user_map_by_name.erase (name);
}

void
CUserManager::RemoveUser (muduo::net::TcpConnection * pTcpConnection)
{
  muduo::MutexLockGuard lock (mutex_);
  m_user_map_by_TcpConnectionAddr.erase (pTcpConnection);
}

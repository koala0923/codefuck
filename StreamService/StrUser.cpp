#include "StrUser.h"

CUser::CUser (const std::string username) :
    m_user_name (username)
{
  m_tcp_connect = NULL;
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
CUserManager::AddUser (std::string name, CUser* user)
{
  if (GetUser (name) == NULL)
    {
      m_user_map_by_name[name] = user;
    }
}

void
CUserManager::AddUser (int fd, CUser* user)
{
  if (GetUser (fd) == NULL)
    {
      m_user_map_by_fd[fd] = user;
    }
}

CUser*
CUserManager::GetUser (std::string name)
{
  CUser* pUser = NULL;
  UserMapbyName_t::iterator it = m_user_map_by_name.find (name);
  if (it != m_user_map_by_name.end ())
    {
      pUser = it->second;
    }
  return pUser;
}

CUser*
CUserManager::GetUser (int fd)
{
  CUser* pUser = NULL;
  UserMapbyFd_t::iterator it = m_user_map_by_fd.find (fd);
  if (it != m_user_map_by_fd.end ())
    {
      pUser = it->second;
    }
  return pUser;
}

void
CUserManager::RemoveUser (std::string name)
{
  m_user_map_by_name.erase (name);
}

void
CUserManager::RemoveUser (int fd)
{
  m_user_map_by_fd.erase (fd);
}

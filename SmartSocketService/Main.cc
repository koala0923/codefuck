#include<stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <muduo/base/Logging.h>
#include <muduo/base/AsyncLogging.h>
#include <muduo/net/EventLoop.h>
#include "SmartSocketService.h"
#include "ConnectionPool.h"
#include "OracleConnection.h"
#include "Config.h"
#include "Message.h"

using namespace muduo;
using namespace muduo::net;

static void
ctrlcHandle (int signum)
{

  LOG_DEBUG << "ctrl-c signal .....";
  exit (0);

}

int kRollSize = 500 * 1000 * 1000;

boost::scoped_ptr<muduo::AsyncLogging> g_asyncLog;

void
asyncOutput (const char* msg, int len)
{
  g_asyncLog->append (msg, len);
}

void
setLogging (const char* argv0)
{
  muduo::Logger::setOutput (asyncOutput);
  char name[256];
  strncpy (name, argv0, 256);
  g_asyncLog.reset (new muduo::AsyncLogging (::basename (name), kRollSize));
  g_asyncLog->start ();
}

int
main (int argc, char* argv[])
{
//  setLogging(argv[0]);
  Config Sconfig ("service.conf");

  struct sigaction act;
  act.sa_handler = ctrlcHandle;
  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  sigaction (SIGINT, &act, NULL);
  char* str_daemon = Sconfig.GetConfigName ("Daemon");
  int is_daemon = atoi (str_daemon);
  if (is_daemon == 1)
    {
      if (0 != daemon (0, 0))
	{

	  LOG_ERROR<<"failed to call daemon():" << strerror(errno);
	  ::abort ();
	}
    }
  char* str_listen_port = Sconfig.GetConfigName ("ListenPort");
  char* mysql_host = Sconfig.GetConfigName ("MySqlhost");
  char* mysql_port = Sconfig.GetConfigName ("MySqlport");
  char* data_base = Sconfig.GetConfigName ("Database");
  char* mysql_user = Sconfig.GetConfigName ("MySqluser");
  char* mysql_password = Sconfig.GetConfigName ("MySqlpassword");
  char* str_mysql_conn_maxsize = Sconfig.GetConfigName ("MySqlconnmaxsize");


  char* str_workthread_size = Sconfig.GetConfigName ("WorkThreadsize");
  char* str_idleSeconds = Sconfig.GetConfigName ("IdleSeconds");
  char* str_PowerThreshold = Sconfig.GetConfigName ("PowerThreshold");


  char* oracle_host = Sconfig.GetConfigName ("Oraclehost");
  char* oracle_sid = Sconfig.GetConfigName ("Oraclesid");
  char* oracle_user = Sconfig.GetConfigName ("Oracleuser");
  char* oracle_password = Sconfig.GetConfigName ("Oraclepassword");
  char* str_oracle_maxConn =  Sconfig.GetConfigName ("OraclemaxConn");
  char* str_oracle_minConn = Sconfig.GetConfigName ("OracleminConn");

  int listen_port = atoi (str_listen_port);

  int mysql_conn_maxsize = atoi (str_mysql_conn_maxsize);

  int workthread_size = atoi (str_workthread_size);
  int idleSeconds = atoi (str_idleSeconds);
  double PowerThreshold_ = atof (str_PowerThreshold);

   int oracle_maxConn = atoi(str_oracle_maxConn);
   int oracle_minConn = atoi(str_oracle_minConn);

  SocketStatuManager::Instance()->Setpowerthreshold(PowerThreshold_);
  LOG_INFO << "Service listen on port = " << listen_port << " pid = "
      << getpid ();



//  if (ConnectionPool::Instance ()->Init (mysql_host, mysql_port, data_base,
//					 mysql_user, mysql_password,
//					 mysql_conn_maxsize) < 0)
//    {
//      LOG_ERROR<<"Failed to init MysqlConnect pool:" << strerror(errno);
//      return -1;
//    }

  if (OracleConnection::Instance ()->Init (oracle_host, oracle_sid, oracle_user,
					   oracle_password, oracle_maxConn,
					   oracle_minConn) < 0)
     {
       LOG_ERROR<<"Failed to init MysqlConnect pool:" << strerror(errno);
       return -1;
     }
  EventLoop loop;
  InetAddress listenAddr (listen_port);
  SmartSocketService server (&loop, listenAddr, workthread_size, idleSeconds);
//  loop.runEvery(1.0, boost::bind(&SmartSocketService::onTimer, &server));
  server.start ();
  loop.loop ();
}


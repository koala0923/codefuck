/*
 * StrService.cpp
 *
 *  Created on: 2016年5月11日
 *      Author: koala
 */

#include<stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "log4z.h"
#include"TcpService.h"
#include "UdpService.h"
#include "TcpConnect.h"
#include "Config.h"

static void
ctrlcHandle (int signum)
{

  LOGD("ctrl-c signal .....");
  exit (0);

}

using namespace zsummer::log4z;
int
main (int argc, char** argv)
{

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

	  LOGE("failed to call daemon():" << strerror(errno));
	  ::abort ();
	}
    }
  zsummer::log4z::ILog4zManager::getRef ().start ();
  zsummer::log4z::ILog4zManager::getRef ().setLoggerLevel (LOG4Z_MAIN_LOGGER_ID,
							   LOG_LEVEL_TRACE);

  char* str_tcp_listen_port = Sconfig.GetConfigName ("TcpListenPort");
  char* str_udp_listen_port = Sconfig.GetConfigName ("UdpListenPort");
  LOGD(
      "TcpListenPort = "<< Sconfig.GetConfigName ("TcpListenPort") << ", UdpListenport = " << Sconfig.GetConfigName ("UdpListenPort"));
  int tcp_listen_port = atoi (str_tcp_listen_port);
  int udp_listen_port = atoi (str_udp_listen_port);

  TcpService Service (tcp_listen_port);
  UdpService UService (udp_listen_port);
  UService.Start ();
  Service.Start ();
  EventLoop::Instance ()->runAfter (20, TcpConnect::timeout_cb);
  EventLoop::Instance ()->StartLoop ();

  return 0;
}


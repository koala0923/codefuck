#include<stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <muduo/base/Logging.h>
#include <muduo/base/AsyncLogging.h>
#include <muduo/net/EventLoop.h>
#include "YFQAppServer.h"

int
main (int argc, char* argv[])
{
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr (9998);
  YFQAppServer server (&loop, listenAddr, 3, 60*3);
  server.start ();
  loop.loop ();
}

#include "YFQAppServer.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

YFQAppServer::YFQAppServer (
    muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr,int numThreads, int idleSeconds) :
    server_ (loop, listenAddr, "YFQAppServer"),workthreadpool_(numThreads),
    codec_ (
	boost::bind (&YFQAppServer::onYFQAppMessage, this, _1, _2,
		     _3))
{
  server_.setConnectionCallback (
      boost::bind (&YFQAppServer::onConnection, this, _1));
  server_.setMessageCallback (
      boost::bind (&ProtolCodec::onMessage, &codec_, _1, _2, _3));

  loop->runEvery(1.0, boost::bind(&YFQAppServer::onTimer, this));
  connectionBuckets_.resize(idleSeconds);

}

void
YFQAppServer::start ()
{
  server_.start ();
  workthreadpool_.Start();
}

void
YFQAppServer::onConnection (const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "YFQAppServer - " << conn->peerAddress ().toIpPort ()
      << " -> " << conn->localAddress ().toIpPort () << " is "
      << (conn->connected () ? "UP" : "DOWN");
  if (conn->connected ())
    {
      LOG_INFO << "Connect!!!!";
	  EntryPtr entry(new Entry(conn));
	  connectionBuckets_.back().insert(entry);
	  WeakEntryPtr weakEntry(entry);
	  conn->setContext(weakEntry);
    }
  else
    {
      LOG_INFO << "Disconnect!!!!";
      assert(!conn->getContext().empty());
	  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
	  LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();

	  muduo::net::TcpConnection *addr = conn.get(); /*获取TcpConnection 的原始地址*/
	  assert(addr != NULL);
	  CUserPtr  user;
	  if ((user = CUserManager::Instance ()->GetUser (addr)))
	    {
	      CUserManager::Instance ()->RemoveUser (addr);
	      CUserManager::Instance ()->RemoveUser (user->GetUsername ());

	    }
    }

}



void
YFQAppServer::onYFQAppMessage (const muduo::net::TcpConnectionPtr& conn,
					  MessagePtr& message,
					  muduo::Timestamp time)
{
  //LOG_INFO << "onSmartSocketMessage!!!!";
  WeakTcpConnectionPtr weak_conn(conn);
  message->setContext(weak_conn, time);
  workthreadpool_.Put(message);
  assert(!conn->getContext().empty());
  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
  EntryPtr entry(weakEntry.lock());
  if(entry)
    {
      connectionBuckets_.back().insert(entry);
    }
}


void YFQAppServer::onTimer(void)
{
    connectionBuckets_.push_back(Bucket());

}



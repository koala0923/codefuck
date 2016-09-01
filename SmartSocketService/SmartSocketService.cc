#include"SmartSocketService.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

SmartSocketService::SmartSocketService (
    muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr,int numThreads, int idleSeconds) :
    server_ (loop, listenAddr, "SmartSocketService"),workthreadpool_(numThreads),
    codec_ (
	boost::bind (&SmartSocketService::onSmartSocketMessage, this, _1, _2,
		     _3))
{
  server_.setConnectionCallback (
      boost::bind (&SmartSocketService::onConnection, this, _1));
  server_.setMessageCallback (
      boost::bind (&ProtolCodec::onMessage, &codec_, _1, _2, _3));

  loop->runEvery(1.0, boost::bind(&SmartSocketService::onTimer, this));
  connectionBuckets_.resize(idleSeconds);
  dumpConnectionBuckets();
}

void
SmartSocketService::start ()
{
  server_.start ();
  workthreadpool_.Start();
}

void
SmartSocketService::onConnection (const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "SmartSocketService - " << conn->peerAddress ().toIpPort ()
      << " -> " << conn->localAddress ().toIpPort () << " is "
      << (conn->connected () ? "UP" : "DOWN");
  if (conn->connected ())
    {
      LOG_INFO << "Connect!!!!";
	  EntryPtr entry(new Entry(conn));
	  connectionBuckets_.back().insert(entry);
	  dumpConnectionBuckets();
	  WeakEntryPtr weakEntry(entry);
	  conn->setContext(weakEntry);
    }
  else
    {
      LOG_INFO << "Disconnect!!!!";
      assert(!conn->getContext().empty());
	WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
	LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
    }

}

//void
//SmartSocketService::onSmartSocketMessage (const muduo::net::TcpConnectionPtr& conn,
//					  Message* message,
//					  muduo::Timestamp)
//{
//  //LOG_INFO << "onSmartSocketMessage!!!!";
//  workthreadpool_.Put(message);
//  assert(!conn->getContext().empty());
//  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
//  EntryPtr entry(weakEntry.lock());
//  if(entry)
//    {
//      connectionBuckets_.back().insert(entry);
//      dumpConnectionBuckets();
//    }
//}

void
SmartSocketService::onSmartSocketMessage (const muduo::net::TcpConnectionPtr& conn,
					  MessagePtr& message,
					  muduo::Timestamp)
{
  //LOG_INFO << "onSmartSocketMessage!!!!";
  workthreadpool_.Put(message);
  assert(!conn->getContext().empty());
  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
  EntryPtr entry(weakEntry.lock());
  if(entry)
    {
      connectionBuckets_.back().insert(entry);
      dumpConnectionBuckets();
    }
}


void SmartSocketService::onTimer(void)
{
    connectionBuckets_.push_back(Bucket());
    dumpConnectionBuckets();
}


void SmartSocketService::dumpConnectionBuckets() const
{
//  LOG_INFO << "size = " << connectionBuckets_.size();
//  int idx = 0;
//  for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
//      bucketI != connectionBuckets_.end();
//      ++bucketI, ++idx)
//  {
//    const Bucket& bucket = *bucketI;
//   // printf("[%d] len = %zd : ", idx, bucket.size());
//    for (Bucket::const_iterator it = bucket.begin();
//        it != bucket.end();
//        ++it)
//    {
//      bool connectionDead = (*it)->weakConn_.expired();
//      printf("%p(%ld)%s, ", get_pointer(*it), it->use_count(),
//          connectionDead ? " DEAD" : "");
//    }
//    puts("");
//  }
}

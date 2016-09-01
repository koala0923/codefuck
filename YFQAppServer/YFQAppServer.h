#ifndef  YFQ_APP_SERVER_H
#define	 YFQ_APP_SERVER_H
#include <muduo/net/TcpServer.h>
#include "ProtolCodec.h"
#include "WorkThreadPool.h"

#include <boost/circular_buffer.hpp>
#include <boost/unordered_set.hpp>
#include <boost/version.hpp>

class YFQAppServer
{
public:
  YFQAppServer (muduo::net::EventLoop* loop,
		const muduo::net::InetAddress& listenAddr, int numThreads,
		int idleSeconds);
  void
  start ();

private:
  void
  onConnection (const muduo::net::TcpConnectionPtr& conn);

  void
  onYFQAppMessage (const muduo::net::TcpConnectionPtr&, MessagePtr& message,
		   muduo::Timestamp);
  void
  onTimer ();

  typedef boost::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;
  struct Entry : public muduo::copyable
  {
    explicit
    Entry (const WeakTcpConnectionPtr & weakConn) :
	weakConn_ (weakConn)
    {

    }

    ~Entry ()
    {
      muduo::net::TcpConnectionPtr conn = weakConn_.lock ();
      if (conn)
	{
	  conn->shutdown ();
	}
    }
    WeakTcpConnectionPtr weakConn_;
  };
  typedef boost::shared_ptr<Entry> EntryPtr;
  typedef boost::weak_ptr<Entry> WeakEntryPtr;
  typedef boost::unordered_set<EntryPtr> Bucket;
  typedef boost::circular_buffer<Bucket> WeakConnectionList;
  muduo::net::TcpServer server_;
  ProtolCodec codec_;
  WorkThreadPool workthreadpool_;
  WeakConnectionList connectionBuckets_;
};

#endif

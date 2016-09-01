#ifndef WORK_THREAD_POLL_H
#define WORK_THREAD_POLL_H

#include <muduo/base/BlockingQueue.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <string>
#include <stdio.h>


class Message;
typedef boost::shared_ptr<Message> MessagePtr;

class WorkThreadPool
{
public:
  //WorkThreadPool (int numThreads);

  WorkThreadPool(int numThreads);

  void
  Start ();
//  void
//  Put ( Message* pmessage);

  void
    Put (MessagePtr pmessage);
  void
  joinAll ();
private:

  void
  threadFunc ();
private:
  bool running_;
//  muduo::BlockingQueue<Message*> queue_;
  muduo::BlockingQueue<MessagePtr> queue_;
  boost::ptr_vector<muduo::Thread> threads_;
};

//int main(int argc, char* argv[])
//{
//  int threads = argc > 1 ? atoi(argv[1]) : 1;
//
//  Bench t(threads);
//  t.run(10000);
//  t.joinAll();
//}


#endif

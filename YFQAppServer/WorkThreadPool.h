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

#include "Message.h"



class Message;


class WorkThreadPool
{
public:

  WorkThreadPool(int numThreads);

  void
  Start ();

  void
    Put (MessagePtr pmessage);
  void
  joinAll ();
private:

  void
  threadFunc ();
private:
  bool running_;
  muduo::BlockingQueue<MessagePtr> queue_;
  boost::ptr_vector<muduo::Thread> threads_;
};




#endif

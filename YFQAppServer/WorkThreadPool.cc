
#include "WorkThreadPool.h"
#include "Message.h"


WorkThreadPool::WorkThreadPool(int numThreads):
      threads_(numThreads)
  {
    for (int i = 0; i < numThreads; ++i)
    {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.push_back(new muduo::Thread(
            boost::bind(&WorkThreadPool::threadFunc, this), muduo::string(name)));
    }

    running_ = true;
  }

void WorkThreadPool::Start(void)
{

  for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::start, _1));

}


void WorkThreadPool::Put( MessagePtr pmessage)
{
  queue_.put(pmessage);
}

void WorkThreadPool::joinAll()
  {
  running_ = false;
    for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::join, _1));
  }



void WorkThreadPool::threadFunc()
  {
    printf("tid=%d, %s started\n",
           muduo::CurrentThread::tid(),
           muduo::CurrentThread::name());



    while (running_)
    {

	MessagePtr pmessage = queue_.take();
		LOG_INFO << "tid = " << muduo::CurrentThread::tid() << "， "<<muduo::CurrentThread::name()<<" Process.";
		pmessage->process();

    }

    printf("tid=%d, %s stopped\n",
           muduo::CurrentThread::tid(),
           muduo::CurrentThread::name());

  }

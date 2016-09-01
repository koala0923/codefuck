#include "EventLoop.h"
#include <event2/event.h>
#include <math.h>
#include "log4z.h"

EventLoop* EventLoop::m_pEventLoop = NULL;
struct cb_arg arg; //定时器参数

EventLoop::EventLoop ()
{
  running = false;
  m_pEventbase = event_base_new ();
  if (m_pEventbase == NULL)
    {

      LOGE("Init Eventbase error");
    }
}

EventLoop::~EventLoop ()
{
  event_base_free (m_pEventbase);
}

int
EventLoop::StartLoop ()
{
  running = true;
  int ret;
  //const char *event_base_get_method(const struct event_base *);
  //printf("fuck dispatch is %s\n",event_base_get_method(m_pEventbase));
  ret = event_base_dispatch (m_pEventbase);
  return ret;
}

struct event *
EventLoop::runAfter (double delay, event_callback_fn fuc)
{
  struct event *timeout = NULL;
  double precious = 0.0, inter = 0;
  precious = modf (delay, &inter);
  //printf("int = %f, pre = %f\n", inter, precious);
  long int seconds = (long int) inter;			//秒
  long int micseconds = precious * 1000 * 1000; 		//微秒
  //printf("second = %ld , micsecond = %ld\n", seconds, micseconds);
  struct timeval tv =
    { seconds, micseconds };

  timeout = evtimer_new(m_pEventbase, fuc, &arg);
  arg.ev = timeout;
  arg.tv = tv;
  evtimer_add(timeout, &tv);
  return timeout;
}

void
EventLoop::cancle (struct event* ev)
{
  evtimer_del(ev);
}

EventLoop*
EventLoop::Instance ()
{
  if (m_pEventLoop == NULL)
    {
      m_pEventLoop = new EventLoop ();
    }

  return m_pEventLoop;
}


#ifndef EVNETLOOP_H_
#define EVNETLOOP_H_

#include<event.h>

struct cb_arg
{
  struct event *ev;
  struct timeval tv;
};

class EventLoop
{
public:
  virtual
  ~EventLoop ();
  static EventLoop*
  Instance ();
  int
  StartLoop ();
  struct event *
  runAfter (double delay, event_callback_fn fuc);
  void
  cancle (struct event *);
  event_base *
  GetEventBase ()
  {
    return m_pEventbase;
  }
  ;
protected:
  EventLoop ();

private:
  static EventLoop* m_pEventLoop;
  event_base *m_pEventbase;
  bool running;
};

#endif

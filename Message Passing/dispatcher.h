#ifndef _MESSAGING_DISPATCHER_H_
#define _MESSAGING_DISPATCHER_H_

#include "message_queue.h"
#include "template_dispatcher.h"

namespace messaging {

class CloseQueue {};

class Dispatcher {
public:
  Dispatcher(Dispatcher&& rhs) :
    q(rhs.q),
    chained(rhs.chained) {
    rhs.chained = false;
  }

  explicit Dispatcher(MessageQueue *q_) :
    q(q_),
    chained(false) {
  }

  template<typename Msg, typename Func>
  TemplateDispatcher<Dispatcher, Msg, Func>
  Handle(Func&& f) {
    return TemplateDispatcher<Dispatcher, Msg, Func>(q, this, 
      std::forward<Func>(f));
  }

  ~Dispatcher() noexcept(false) {
    if (!chained) {
      WaitAndDispatch();
    }
  }

private:
  MessageQueue *q;
  bool chained;
  Dispatcher(const Dispatcher&) = delete;
  Dispatcher& operator=(const Dispatcher&) = delete;

  template<typename Dispatch, typename Msg, typename Func>
  friend class TemplateDispatcher;

  void WaitAndDispatch() {
    for (;;) {
      auto msg = q->WaitAndPop();
      Dispatch(msg);
    }
  }

  // If the message indicates a CloseMessage, throw an exception. Otherwise,
  // return false indicating the message unhandled.
  bool Dispatch(const std::shared_ptr<MessageBase>& msg) {
    if (dynamic_cast<WrappedMessage<CloseQueue>*>(msg.get())) {
      throw CloseQueue();
    }
    return false;
  }
};

}

#endif // _MESSAGING_DISPATHER_H_
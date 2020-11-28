#ifndef _MESSAGING_TEMPLATE_DISPATCHER_H_
#define _MESSAGING_TEMPLATE_DISPATCHER_H_

#include "message_queue.h"

namespace messaging {

template<typename PrevDispatcher, typename Msg, typename Func>
class TemplateDispatcher {
public:
  TemplateDispatcher(TemplateDispatcher&& rhs) :
    q(rhs.q),
    prev(rhs.prev),
    chained(rhs.chained) 
  {
    rhs.chained = false;    
  }

  TemplateDispatcher(MessageQueue *q_, PrevDispatcher *prev_, Func&& f_) :
    q(q_),
    prev(prev_),
    f(std::forward<Func>(f_)),
    chained(false) 
  {
    prev_->chained = false;
  }

  template<typename OtherMsg, typename OtherFunc, typename>
  TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>
  Handle(OtherFunc&& f_) {
    return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(
      q, this, std::forward<OtherFunc>(f_)
    );
  }

  ~TemplateDispatcher() noexcept(false) {
    if (!chained) {
      WaitAndDispatch();
    }
  }

private:
  MessageQueue *q;
  PrevDispatcher *prev;
  Func f;
  bool chained;
  TemplateDispatcher(const TemplateDispatcher&) = delete;
  TemplateDispatcher& operator=(const TemplateDispatcher&) = delete;

  // Instantiation of TemplateDispatcher is friend of each other.
  template<typename OtherDispatcher, typename OtherMsg, typename OtherFunc>
  friend class TemplateDispatcher;

  void WaitAndDispatch() {
    for (;;) {
      auto msg = q->WaitAndPop();
      if (Dispatch(msg)) {
        break;
      }
    }
  }

  // If the message type matches the current dispatcher, handles it. Otherwise,
  // pass it to previous dispatcher.
  bool Dispatch(const std::shared_ptr<MessageBase>& msg) {
    if (WrappedMessage<Msg> *wrapper = 
      dynamic_cast<WrappedMessage<Msg>*>(msg.get())) 
    {
      f(wrapper->content);
      return true;
    } else {
      return prev->Dispatch(msg);
    }
  }
};

} // namespace messaging

#endif // _MESSAGING_TEMPLATE_DISPATCHER_H_
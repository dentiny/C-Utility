#ifndef _MESSAGING_SENDER_H_
#define _MESSAGING_SENDER_H_

#include "message_queue.h"

namespace messageing {

class Sender {
public:
  Sender() : 
    q(nullptr) {
  }

  explicit Sender(MessgageQueue *q_) :
    q(q_) {
  }

  template<typname Msg>
  void Send(const Msg& msg) {
    if (q != nullptr) {
      q->push(msg);
    }
  }

private:
  MessageQueue* q;
};

}; // namespace messaging

#endif // _MESSAGING_SENDER_H_
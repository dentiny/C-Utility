#ifndef _MESSAGING_RECEIVER_H_
#define _MESSAGING_RECEIVER_H_

#include "message_queue.h"

namespace messaging {

class Receiver {
public:
  // Allow implicit conversion to Sender which references the queue.
  operator Sender() {
    return Sender(q);
  }

  Dispatcher wait() {
    return Dispatcher(q);
  }

private:
  MessageQueue *q;
};

} // namespace messaging

#endif // _MESSAGING_RECEIVER_H_
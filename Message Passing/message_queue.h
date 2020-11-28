#ifndef _MESSAGING_MESSAGE_QUEUE_H_
#define _MESSAGING_MESSAGE_QUEUE_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace messaging {

class MessageBase {
public:
  virtual ~MessageBase() {
  }
};

template <typename Msg>
class WrappedMessage : private MessageBase {
public:
  Msg content;
  explicit WrappedMessage(const Msg& content_) :
    content(content_) {
  }
};

class MessageQueue {
public:
  template<typename Msg>
  void Push(const Msg& msg) {
    std::unique_lock<std::mutex> lck(mtx);
    q.push(std::make_shared<WrappedMessage<Msg>>(msg));
    cv.notify_all();
  }

  std::shared_ptr<MessageBase> WaitAndPop() {
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [&]() { return !q.empty(); });
    auto res = q.front();
    q.pop();
    return res;
  }

private:
  std::mutex mtx;
  std::condition_variable cv;
  std::queue<std::shared_ptr<MessageBase>> q;
};

} // namespace messaging

#endif // _MESSAGING_MESSAGE_QUEUE_H_
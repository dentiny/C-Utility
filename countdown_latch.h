#ifndef COUNTDOWN_LATCH
#define COUNTDOWN_LATCH

#include <condition_variable>
#include <mutex>

using std::condition_variable;
using std::mutex;
using std::unique_lock;

class CountdownLatch {
 public:
  explicit CountdownLatch(int count) :
    count_(count) {}

  void Wait() {
    unique_lock<mutex> lck(mtx_);
    cv_.wait(lck, [this]() { return count_ == 0; });
  }

  void Countdown() {
    unique_lock<mutex> lck(mtx_);
    if (--count_ == 0) {
      cv_.notify_one();
    }
  }

 private:
  int count_;
  mutable mutex mtx_;
  condition_variable cv_;
};

#endif  // COUNTDOWN_LATCH

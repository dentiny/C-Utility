#ifndef _BOUNDED_BUFFER_QUEUE_H_
#define _BOUNDED_BUFFER_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class BoundedBufferQueue {
public:
	BoundedBufferQueue(unsigned int cap) :
		cap_(cap),
		stop_(false),
		get_counter_(0),
		push_counter_(0) {
	}

	BoundedBufferQueue(const BoundedBufferQueue&) = delete;
	BoundedBufferQueue& operator=(const BoundedBufferQueue&) = delete;
	BoundedBufferQueue(BoundedBufferQueue&&) = delete;
	BoundedBufferQueue& operator=(BoundedBufferQueue&&) = delete;

	void GetFront(T *val) {
		++get_counter_;
		std::unique_lock<std::mutex> lck(mtx_);
		cv1_.wait(lck, [&]() { return stop_ || !queue_.empty(); });
		if (stop_) {
			return;
		}
		*val = queue_.front(); queue_.pop();
		--get_counter_;
		cv2_.notify_one();
	}

  bool IsEmpty() const {
    std::lock_guard<std::mutex> lck(mtx_);
    return queue_.empty();
  }

  int Size() const {
    std::lock_guard<std::mutex> lck(mtx_);
    return static_cast<int>(queue_.size());
  }

	void Push(T val) {
		++push_counter_;
		std::unique_lock<std::mutex> lck(mtx_);
		cv2_.wait(lck, [&]() { return stop_ || queue_.size() < cap_; });
		if (stop_) {
			return;
		}
		queue_.push(val);
		--push_counter_;
		cv1_.notify_one();
	}

	~BoundedBufferQueue() noexcept {
		stop_ = true;
		cv1_.notify_all();
		cv2_.notify_all();
		while (get_counter_ > 0 || push_counter_ > 0);
	}

private:
	unsigned cap_;
	bool stop_;
	mutable std::mutex mtx_;
	std::queue<T> queue_;
	std::atomic<int> get_counter_; // number of blocked get() thread
	std::atomic<int> push_counter_; // number of blocked set() thread
	std::condition_variable cv1_; // used in consumer, to notify producer
	std::condition_variable cv2_; // used in producer, to notify consumer
};

#endif // _BOUNDED_BUFFER_QUEUE_H_
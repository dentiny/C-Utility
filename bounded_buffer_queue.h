#ifndef _BOUNDED_BUFFER_QUEUE_H_
#define _BOUNDED_BUFFER_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class BoundedBufferQueue {
public:
	BoundedBufferQueue(unsigned int cap_) :
		cap { cap_ },
		stop { false },
		get_counter { 0 },
		push_counter { 0 } {
	}

	BoundedBufferQueue(const BoundedBufferQueue&) = delete;
	BoundedBufferQueue& operator=(const BoundedBufferQueue&) = delete;
	BoundedBufferQueue(BoundedBufferQueue&&) = delete;
	BoundedBufferQueue& operator=(BoundedBufferQueue&&) = delete;

	void get() {
		++get_counter;
		std::unique_lock<std::mutex> lck(mtx);
		cv1.wait(lck, [&]() { return stop || !q.empty(); });
		if (stop) {
			return;
		}
		T val = q.front(); q.pop();
		--get_counter;
		cv2.notify_one();
	}

	void push(int val) {
		++push_counter;
		std::unique_lock<std::mutex> lck(mtx);
		cv2.wait(lck, [&]() { return stop || q.size() < cap; });
		if (stop) {
			return;
		}
		q.push(val);
		--push_counter;
		cv1.notify_one();
	}

	~BoundedBufferQueue() noexcept {
		stop = true;
		cv1.notify_all();
		cv2.notify_all();
		while (get_counter > 0 || push_counter > 0);
	}

private:
	unsigned cap;
	bool stop;
	std::mutex mtx;
	std::queue<T> q;
	std::atomic<int> get_counter; // number of blocked get() thread
	std::atomic<int> push_counter; // number of blocked set() thread
	std::condition_variable cv1; // used in consumer, to notify producer
	std::condition_variable cv2; // used in producer, to notify consumer
};

#endif // _BOUNDED_BUFFER_QUEUE_H_
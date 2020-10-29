#ifndef _RWLOCK_
#define _RWLOCK_

#include <atomic>
#include <condition_variable>
#include <mutex>

class RwLock {
public:
	RwLock() :
		counter { 0 } {
	}

	void AcquireWrite() {
		std::unique_lock<std::mutex> lck(mtx);
		wCv.wait(lck, [&]() { return counter == 0; });
		--counter;
	}

	void ReleaseWrite() {
		std::unique_lock<std::mutex> lck(mtx);
		counter = 0;
		rCv.notify_all();
	}

	void AcquireRead() {
		std::unique_lock<std::mutex> lck(mtx);
		rCv.wait(lck, [&]() { return counter >= 0; });
		++counter;
	}

	void ReleaseRead() {
		std::unique_lock<std::mutex> lck(mtx);
		--counter;
		wCv.notify_one();
	}

private:
	// The global lock.
	std::mutex mtx;

	// CV for read and write.
	std::condition_variable rCv;
	std::condition_variable wCv;

	// Counter to represent rwlock state.
	// 0 for free, positive for reader share, -1 for write exclusive.
	std::atomic<int> counter;
};

#endif // _RWLOCK_
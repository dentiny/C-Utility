#include "thread.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <ucontext.h>
using namespace std;

class Thread {
public:
  Thread(ucontext_t* context_) :
    context(context_) {
    assert(context != NULL);
  }

  ~Thread() {
    assert(context != NULL);
    delete[] (char*)context->uc_stack.ss_sp;
    delete context;
  }

public:
  ucontext_t* context;

private:
  Thread(const Thread&);
  Thread& operator=(const Thread&);
};

// Global variables to represent the state of coroutine lib.
static bool already_inited = false;
static Thread* main_thread = NULL;
static Thread* running_thread = NULL;
static queue<Thread*> ready_threads;
static map<unsigned int, Thread*> locks_held;
static map<unsigned int, queue<Thread*> > lock_blocked_threads;
static map<pair<int, int>, queue<Thread*> > monitor_blocked_threads;

// Decide whether there's thread blocked by this lock.
static bool has_thread_blocked_by_lock(unsigned int lock) {
  return !lock_blocked_threads[lock].empty();
}

// Decide whether there's thread blocked by this monitor.
static bool has_thread_blocked_by_monitor(pair<unsigned int, unsigned int>& monitor) {
  return !monitor_blocked_threads[monitor].empty();
}

static Thread* fetch_ready_thread() {
  assert(!ready_threads.empty());
  Thread* new_thread = ready_threads.front();
  ready_threads.pop();
  assert(new_thread != NULL);
  return new_thread;
}

static Thread* fetch_lock_blocked_thread(unsigned int lock) {
  queue<Thread*>& lock_blocked_queue = lock_blocked_threads[lock];
  assert(!lock_blocked_queue.empty());
  Thread* new_thread = lock_blocked_queue.front();
  lock_blocked_queue.pop();
  return new_thread;
}

static Thread* fetch_monitor_blocked_thread(pair<unsigned int, unsigned int>& monitor) {
  queue<Thread*>& monitor_blocked_queue = monitor_blocked_threads[monitor];
  assert(!monitor_blocked_queue.empty());
  Thread* new_thread = monitor_blocked_queue.front();
  monitor_blocked_queue.pop();
  return new_thread;
}

static ucontext_t* create_new_context() {
  ucontext_t *new_context = new (nothrow) ucontext_t;
  if (new_context == NULL) {
    cerr << "Out of mem: create new context failure" << endl;
    return NULL;
  }
  char* stk = new (nothrow) char[STACK_SIZE];
  if (stk == NULL) {
    delete new_context;
    cerr << "Out of mem: create new stack error" << endl;
    return NULL;
  }

  // Except for main_thread, every time a thread finishes its execution, it
  // switches back to main_thread.
  getcontext(new_context);
  new_context->uc_stack.ss_sp = stk;
  new_context->uc_stack.ss_size = STACK_SIZE;
  new_context->uc_stack.ss_flags = 0;
  new_context->uc_link = NULL;
  return new_context;
}

// Execute the user program, and switch context after execution.
static void thread_wrapper(thread_startfunc_t func, void *arg) {
  func(arg);

  // Switch to main_thread and context switch.
  swapcontext(running_thread->context, main_thread->context);
}

// Called at (1) thread_libinit() for creating the first thread.
// (2) thread_create().
static int thread_create_helper(thread_startfunc_t func, void *arg) {
  // Create context for new thread.
  ucontext_t *new_context = create_new_context();
  if (new_context == NULL) {
    return -1;
  }
  makecontext(new_context, (void(*)())thread_wrapper, 2, func, arg);

  // Initialize new thread.
  Thread *new_thread = new (nothrow) Thread(new_context);
  if (new_thread == NULL) {
    delete[] (char*)new_context->uc_stack.ss_sp;
    delete new_context;
    cerr << "Out of mem: create new thread error" << endl;
    return -1;
  }
  ready_threads.push(new_thread);
  
  return 0;
}

int thread_libinit(thread_startfunc_t func, void *arg) {
  // Already initialized, return error.
  if (already_inited) {
    cerr << "Thread library already initialized" << endl;
    return -1;
  }

  // Create main thread.
  already_inited = true;
  ucontext_t* main_context = create_new_context();
  if (main_context == NULL) {
    return -1;
  }
  main_thread = new (nothrow) Thread(main_context);
  if (main_thread == NULL) {
    cerr << "Out of mem: create main thread failure" << endl;
    return -1;
  }

  // Create the start thread.
  if (thread_create_helper(func, arg) == -1) {
    return -1;
  }

  // Fetch ready threads one by one and execute, until there's no ready thread.
  while (!ready_threads.empty()) {
    // Previous thread has finished execution, memory deallocation.
    if (running_thread != NULL) {
      delete running_thread;
      running_thread = NULL;
    }

    // Fetch and execute new thread, while storing current context in the main 
    // thread. If the thread is awaken by signal/broadcast, then try to acquire
    // lock first.
    running_thread = fetch_ready_thread();
    swapcontext(main_thread->context, running_thread->context);
  }

  // Exit the thread library.
  delete main_thread;
  cout << "Thread library exiting." << endl;
  exit(EXIT_SUCCESS);
}

int thread_create(thread_startfunc_t func, void *arg) {
    // Check whether the thread library has been initialized.
    if (!already_inited) {
      cerr << "Create thread when the thread library hasn't been initialized" << endl;
      return -1;
    }

    int ret = thread_create_helper(func, arg);
    return ret;
}

int thread_yield() {
  // Check if the thread library has been initialized.
  if (!already_inited) {
    cerr << "Yield thread when the lib hasn't been initialized" << endl;
    return -1;
  }

  assert(running_thread != NULL);

  // Empty ready queue, have nothing to do.
  if (ready_threads.empty()) {
    return 0;
  }

  Thread* new_thread = fetch_ready_thread();
  Thread* cur_thread = running_thread;
  ready_threads.push(cur_thread);
  running_thread = new_thread;
  swapcontext(cur_thread->context, new_thread->context);
  return 0;
}

// The running thread will try to acquire the lock. If it could succeed at 
// first try, then it keep running; otherwise, it will be blocked and let other
// ready threads to run first by context switch; it returns back to its current
// context only when another thread gives it the lock via handoff, and put it
// into ready_queue.
static int thread_lock_helper(unsigned int lock) {
  // If the running thread has held the lock, deadlock.
  assert(running_thread != NULL);
  if (locks_held[lock] == running_thread) {
    cerr << "Lock has been held for running thread, deadlock" << endl;
    return -1;
  }

  // If the lock is not held by other threads, simply update the locked state
  // of the running thread, locking action has been completed.
  if (locks_held[lock] == NULL) {
    locks_held[lock] = running_thread;
    return 0;
  }

  // The lock is acquired by one other thread, do context switch. If there's no
  // ready one, exit the program directly.
  if (ready_threads.empty()) {
    cerr << "No ready queue to context switch for acquiring the lock, deadlock" << endl;
    cout << "Thread library exiting." << endl;
    exit(EXIT_SUCCESS);
  }

  // The running thread cannot hold the lock right now, have to switch context 
  // and give control to another thread.
  Thread* new_thread = fetch_ready_thread();
  Thread* cur_thread = running_thread;
  lock_blocked_threads[lock].push(cur_thread);
  running_thread = new_thread;
  swapcontext(cur_thread->context, new_thread->context);
  return 0;
}

int thread_lock(unsigned int lock) {
  // Check if the thread library has been initialized.
  if (!already_inited) {
    cerr << "Lock thread when the lib hasn't been initialized" << endl;
    return -1;
  }

  int ret = thread_lock_helper(lock);
  return ret;
}

// Called at (1) thread_unlock().
// (2) thread_signal(), thread_broadcast().
// Update the locked state of global locks_held set, and block status of running
// thread. Try to get a ready thread which is waiting for the lock, update its
// locking status, and append it into ready queue.
static int thread_unlock_helper(unsigned int lock) {
  // If the lock is not held by any thread, or the running thread doesn't hold
  // that lock.
  assert(running_thread != NULL);
  if (locks_held[lock] != running_thread) {
    cerr << "Lock is not held by the running thread" << endl;
    return -1;
  }

  assert(locks_held[lock] == running_thread);

  // Update the lock status.
  locks_held[lock] = NULL;

  // Fetch the first thread which is lock-blocked by the lock, put it to ready 
  // state. If no thread is blocked by this lock, just return.
  if (!has_thread_blocked_by_lock(lock)) {
    return 0;
  }

  // Fetch and remove one lock-blocked thread from the waiting queue, give it 
  // the lock by handoff, and add it to ready queue.
  Thread* thd = fetch_lock_blocked_thread(lock);
  locks_held[lock] = thd;
  ready_threads.push(thd);
  return 0;
}

int thread_unlock(unsigned int lock) {
  // Check thread library has been initialized.
  if (!already_inited) {
    cerr << "Unlock thread when thread library hasn't initialized" << endl;
    return -1;
  }

  int ret = thread_unlock_helper(lock);
  return ret;
}

// The running thread first unlocks the lock held, then, like lock(), update its
// monitor-related blocking state, then context switch. Only when the corresponding
// monitor is signal() or broadcast() will the current thread return back to the
// current thread. Before execution of logical part, the lock has to be re-acquired.
int thread_wait(unsigned int lock, unsigned int cond) {
  // Check whether the thread library has been initialized.
  if (!already_inited) {
    cerr << "Wait thread when the thread library hasn't been initialized" << endl;
    return -1;
  }

  // Check if the thread library has been initialized, and unlocking is normal. 
  // It could fail when the running thread doesn't hold the lock.
  if (thread_unlock_helper(lock) == -1) {
    return -1;
  }

  // Fetch a ready thread from the ready_queue, and context switch.
  if (ready_threads.empty()) {
    cerr << "No ready queue to context switch after thread_wait, deadlock" << endl;
    cout << "Thread library exiting." << endl;
    exit(EXIT_SUCCESS);
  }

  // Fetch a new thread from ready_queue, and do context switch.
  Thread* new_thread = fetch_ready_thread();
  Thread* cur_thread = running_thread;
  monitor_blocked_threads[make_pair(lock, cond)].push(cur_thread);
  running_thread = new_thread;
  swapcontext(cur_thread->context, new_thread->context);

  // One another thread has waken up the monitor-blocked thread. Before go back
  // to the logical part, lock has to be re-acquired.
  int ret = thread_lock_helper(lock);
  return ret;
}

// Fetch a thread which is blocked by this monitor, and put it into ready queue.
// After the new thread returns to its original context, it will try to contend
// the lock.
int thread_signal(unsigned int lock, unsigned int cond) {
  // Check whether the thread library has been initialized.
  if (!already_inited) {
    cerr << "Signal when the thread library hasn't been initialized" << endl;
    return -1;
  }

  // No thread is blocked by the monitor.
  pair<unsigned int, unsigned int> monitor = make_pair(lock, cond);
  if (!has_thread_blocked_by_monitor(monitor)) {
    return 0;
  }

  // Fetch a thread which is blocked by monitor (lock, cond), and put it into 
  // ready queue.
  Thread* new_thread = fetch_monitor_blocked_thread(monitor);
  ready_threads.push(new_thread);
  return 0;
}

// Fetch all threads which are blocked by this monitor, and put it into ready 
// queue. After return back to the original contexts, all threads will contend
// for one free lock.
int thread_broadcast(unsigned int lock, unsigned int cond) {
  // Check whether the thread library has been initialized.
  if (!already_inited) {
    cerr << "Broadcast when the thread library hasn't been initialized" << endl;
    return -1;
  }

  // Fetch all threads which are blocked by the monitor.
  pair<unsigned int, unsigned int> monitor = make_pair(lock, cond);
  while (has_thread_blocked_by_monitor(monitor)) {
    Thread* new_thread = fetch_monitor_blocked_thread(monitor);
    ready_threads.push(new_thread);
  }
  
  return 0;
}
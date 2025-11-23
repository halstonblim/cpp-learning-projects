#pragma once
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_; // unsafe queue we need to protect
    mutable std::mutex mutex_; // only one thread can unlock mutex. can be modified by const methods.
    std::condition_variable cond_; // wake up consumer

public:
    ThreadSafeQueue() = default;

    // = delete means we don't allow copy or assignment
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // PRODUCER code
    void push(const T& value) {
        // lock mutex
        std::lock_guard<std::mutex> lock(mutex_);

        queue_.push(value);
        cond_.notify_one();
    }

    // CONSUMER
    // blocked until guaranteed to populate value
    void wait_and_pop(T& value) {
        // lock mutex if available. otherwise thread pauses
        std::unique_lock<std::mutex> lock(mutex_);

        // allow mutex to be unlocked, until queue not empty
        // > if true, function returns we keep lock and proceed
        // > if false, mutex is unlocked and thread put to sleep
        //    thread wakes up when producer calls notify_one()
        cond_.wait(lock, [this]{return !queue_.empty();});
        
        value = queue_.front();
        queue_.pop();
    } 

    // CONSUMER
    // immediately returns true/false, never waits
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }

    // check if empty (thread-safe)
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};
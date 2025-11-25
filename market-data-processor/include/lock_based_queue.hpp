#pragma once
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;          // mutable: lockable in const methods
    std::condition_variable cond_;      // signals waiting consumers

public:
    ThreadSafeQueue() = default;

    // Non-copyable, non-assignable
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Producer: push item and notify one waiting consumer
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_.notify_one();
    }

    // Consumer: blocks until item available
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Releases lock while waiting; reacquires when notified and predicate true
        cond_.wait(lock, [this]{return !queue_.empty();});
        value = queue_.front();
        queue_.pop();
    }

    // Consumer: non-blocking, returns false if empty
    [[nodiscard]] bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }

    // Thread-safe empty check
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

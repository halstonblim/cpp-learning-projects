#pragma once
#include <vector>
#include <atomic>
#include <optional>

// SPSC lock-free ring buffer
template<typename T>
class LockFreeQueue {
public:
    explicit LockFreeQueue(size_t size)
        : buffer_(size + 1), head_(0), tail_(0) {}

    bool push(const T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);
        size_t next_head = (current_head + 1) % buffer_.size();

        if (next_head == current_tail) {
            return false;
        }

        buffer_[current_head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    [[nodiscard]] std::optional<T> pop() {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        if (current_head == current_tail) {
            return std::nullopt;
        }

        T item = buffer_[current_tail];
        size_t next_tail = (current_tail + 1) % buffer_.size();
        tail_.store(next_tail, std::memory_order_release);
        return item;
    }

    [[nodiscard]] size_t size() const {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (head >= tail) {
            return head - tail;
        }
        return buffer_.size() - tail + head;
    }

    [[nodiscard]] size_t capacity() const {
        return buffer_.size() - 1;
    }

private:
    std::vector<T> buffer_;
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};

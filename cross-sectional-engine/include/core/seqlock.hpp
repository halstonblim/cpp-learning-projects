#pragma once
#include <atomic>
#include <cstdint>

// Single-writer, multi-reader seqlock
class Seqlock {
public:
    Seqlock() : seq_(0) {}

    void write_lock() {
        seq_.fetch_add(1, std::memory_order_acquire);
    }

    void write_unlock() {
        seq_.fetch_add(1, std::memory_order_release);
    }

    [[nodiscard]] uint64_t read_begin() const {
        uint64_t seq;
        do {
            seq = seq_.load(std::memory_order_acquire);
        } while (seq & 1);
        return seq;
    }

    [[nodiscard]] bool read_retry(uint64_t start_seq) const {
        std::atomic_thread_fence(std::memory_order_acquire);
        return seq_.load(std::memory_order_relaxed) != start_seq;
    }

private:
    alignas(64) std::atomic<uint64_t> seq_;
};
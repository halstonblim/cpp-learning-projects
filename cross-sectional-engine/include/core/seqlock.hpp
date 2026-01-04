#pragma once
#include <atomic>
#include <cstdint>

// single writer, multiple reader seqlock
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
        // spin while odd (write in progress)
        // acquire ensures reads don't start before we load seq
        do {
            seq = seq_.load(std::memory_order_acquire);
        } while (seq & 1);
        return seq;
    }

    [[nodiscard]] bool read_retry(uint64_t start_seq) const {
        // prevent reads from moving past fence
        std::atomic_thread_fence(std::memory_order_acquire);
        // write operations will increment seq to trigger retry
        return seq_.load(std::memory_order_relaxed) != start_seq;
    }
private:
    // intel mac has 64 byte cache, prevents false sharing
    alignas(64) std::atomic<uint64_t> seq_;
};
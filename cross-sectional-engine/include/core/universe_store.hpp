#pragma once
#include "core/aligned_allocator.hpp"
#include "core/seqlock.hpp"
#include <vector>
#include <cstdint>

struct MarketUpdate {
    uint32_t asset_id;
    float price;
    float volume;
    float bid;
    float ask;
};

class UniverseStore {
public:
    explicit UniverseStore(std::size_t capacity)
    : capacity_(capacity)
    {
        prices_.resize(capacity);
        volumes_.resize(capacity);
        bids_.resize(capacity);
        asks_.resize(capacity);
    }

    [[nodiscard]] size_t capacity() const { return capacity_; }   
    [[nodiscard]] const float* get_prices() const { return prices_.data(); }
    [[nodiscard]] const float* get_volumes() const { return volumes_.data(); }
    [[nodiscard]] const float* get_bids() const { return bids_.data(); }
    [[nodiscard]] const float* get_asks() const { return asks_.data(); }

    const Seqlock& seqlock() const {return seqlock_; }
    void write_lock() {seqlock_.write_lock(); }
    void write_unlock() {seqlock_.write_unlock(); }

    // Const reference is crucial here for correctness and flexibility
    inline void update_tick(const MarketUpdate& update) {
        if (update.asset_id >= capacity_) [[unlikely]] {
            return;
        }
        // No bounds checking on vectors for speed (we trusted asset_id above)
        prices_[update.asset_id] = update.price;
        volumes_[update.asset_id] = update.volume;
        bids_[update.asset_id] = update.bid;
        asks_[update.asset_id] = update.ask;
    }

private:
    size_t capacity_;
    // 32-byte alignment for AVX2
    std::vector<float, AlignedAllocator<float, 32>> prices_;
    std::vector<float, AlignedAllocator<float, 32>> volumes_;
    std::vector<float, AlignedAllocator<float, 32>> bids_;
    std::vector<float, AlignedAllocator<float, 32>> asks_;

    Seqlock seqlock_;
};
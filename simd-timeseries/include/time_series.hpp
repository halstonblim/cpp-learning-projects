#pragma once
#include "aligned_allocator.hpp"
#include <vector>
#include <stdexcept>

class TimeSeries {
public:
    TimeSeries(size_t max_capacity) : capacity_(max_capacity), head_(0), is_full_(false) {
        if (max_capacity == 0) {
            throw std::invalid_argument("capacity must be > 0");
        }
        data.resize(max_capacity);
    }
    void add_tick(double price);
    [[nodiscard]] double get_mean() const;
    [[nodiscard]] double get_mean_simd() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;    
    void clear();
    const double* get_data() const; // returns raw poiner to data

private:
    size_t capacity_;
    size_t head_;
    bool is_full_;
    std::vector<double, AlignedAllocator<double>> data;
};
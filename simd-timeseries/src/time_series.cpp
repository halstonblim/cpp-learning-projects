#include "time_series.hpp"
#include "simd_utils.hpp"  // AVX2 math utilities (contains <immintrin.h>)

void TimeSeries::add_tick(double price) {
    data[head_] = price;
    head_++;
    if (head_ == capacity_) {
        is_full_ = true;
        head_ = 0;
    }
}

size_t TimeSeries::size() const {
    return (is_full_ ? capacity_ : head_);
}

double TimeSeries::get_mean() const {
    size_t n = size();
    if (n == 0) return 0.0;
    double s = 0;
    for (size_t i = 0; i < n; ++i) s += data[i];
    return s / n;
}

const double* TimeSeries::get_data() const {
    return data.data();
}

size_t TimeSeries::capacity() const {
    return capacity_;
}

void TimeSeries::clear() {
    head_ = 0;
    is_full_ = false;
}

double TimeSeries::get_mean_simd() const {
    size_t n = size();
    if (n == 0) return 0.0;

    // Delegate to the reusable SIMD utility
    double total_sum = sum_avx2(data.data(), n);
    return total_sum / n;
}
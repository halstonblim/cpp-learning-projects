#include "time_series.hpp"

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
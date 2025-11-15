#pragma once
#include <cstdint>

template <typename RealType>
class OnlineStats {
public:
    OnlineStats() : count_(0), mean_(0), m2_(0) {};
    void Add(RealType value) {
        count_++;
        RealType delta = value - mean_;
        mean_ += delta / count_;
        m2_ += delta * (value - mean_);
    }
    RealType Mean() const {
        return mean_;
    }
    RealType Variance() const {
        return count_ > 0 ? m2_ / count_ : 0;
    }
    RealType SampleVariance() const {
        return count_ > 1 ? m2_ / (count_ - 1) : 0;
    }
    int64_t Count() const {
        return count_;
    };
private:
    int64_t count_;
    RealType mean_;
    RealType m2_;
};
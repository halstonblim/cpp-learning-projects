#pragma once

#include <algorithm>
#include <vector>

namespace derivlib::mc::payoffs {

class EuropeanCall {
public:
    explicit EuropeanCall(double strike) : strike_(strike) {}

    double operator()(const std::vector<double>& path) const {
        return std::max(path.back() - strike_, 0.0);
    }

private:
    double strike_;
};

class EuropeanPut {
public:
    explicit EuropeanPut(double strike) : strike_(strike) {}

    double operator()(const std::vector<double>& path) const {
        return std::max(strike_ - path.back(), 0.0);
    }
private:
    double strike_;
};

} // namespace derivlib::mc::payoffs
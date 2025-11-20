#pragma once

#include <cstdlib>
#include <vector>
#include <random>

namespace derivlib::mc::models {

class IProcessModel {
public:
    virtual ~IProcessModel() = default;

    virtual double generate_path(double expiry, std::vector<double>& path, const std::vector<double>& random_increments) const = 0;
};

}
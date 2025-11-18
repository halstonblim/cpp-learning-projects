#pragma once

#include <cstdlib>
#include <vector>
#include <random>

namespace derivlib::mc::models {

class IProcessModel {
public:
    virtual ~IProcessModel() = default;

    virtual double generate_path(double expiry, std::size_t num_steps, std::vector<double>& path, std::mt19937& rng) const = 0;
};

}
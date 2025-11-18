#pragma once

#include "mc/models/IProcessModel.h"
#include <cstdlib>
#include <vector>
#include <random>

namespace derivlib::mc::models {

class BlackScholesModel : public IProcessModel {
public:
    BlackScholesModel(double spot, double rate, double vol);
    double generate_path(double expiry, std::size_t num_steps, std::vector<double>& path, std::mt19937& rng) const override;

private:
    double spot_;
    double vol_;
    double drift_;
};

}
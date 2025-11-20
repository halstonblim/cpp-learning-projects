#pragma once

#include "mc/models/IProcessModel.h"
#include <cstdlib>
#include <vector>

namespace derivlib::mc::models {

class BlackScholesModel : public IProcessModel {
public:
    BlackScholesModel(double spot, double rate, double vol);
    double generate_path(double expiry, std::vector<double>& path, const std::vector<double>& random_increments) const override;

private:
    double spot_;
    double vol_;
    double drift_;
};

}
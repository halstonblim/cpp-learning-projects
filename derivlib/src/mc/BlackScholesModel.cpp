#include "mc/models/BlackScholesModel.h"
#include <cmath>

namespace derivlib::mc::models {

BlackScholesModel::BlackScholesModel(double spot, double rate, double vol) :
    spot_(spot),
    vol_(vol),
    drift_(rate - 0.5 * vol * vol) {}    

double BlackScholesModel::generate_path(double expiry, std::vector<double>& path, const std::vector<double>& random_increments) const {
        std::size_t num_steps = random_increments.size();
        double dt = expiry / num_steps;
        double drift_dt = drift_ * dt;
        double vol_dt = vol_ * std::sqrt(dt);        
        path[0] = spot_;
        for (std::size_t i = 1; i <= num_steps; ++i) {
            path[i] = path[i-1] * std::exp(drift_dt + vol_dt * random_increments[i-1]);            
        }
        return path[num_steps];
    }

}
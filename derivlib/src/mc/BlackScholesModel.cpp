#include "mc/models/BlackScholesModel.h"
#include <random>
#include <cmath>

namespace derivlib::mc::models {

BlackScholesModel::BlackScholesModel(double spot, double rate, double vol) :
    spot_(spot),
    vol_(vol),
    drift_(rate - 0.5 * vol * vol) {}    

double BlackScholesModel::generate_path(double expiry, std::size_t num_steps, std::vector<double>& path, std::mt19937& rng) const {
        std::normal_distribution<double> normal(0.0, 1.0);
        double dt = expiry / num_steps;
        double drift_dt = drift_ * dt;
        double vol_dt = vol_ * std::sqrt(dt);
        
        path.resize(num_steps + 1);
        path[0] = spot_;
        for (std::size_t i = 1; i <= num_steps; ++i) {
            double Z = normal(rng);            
            path[i] = path[i-1] * std::exp(drift_dt + vol_dt * Z);            
        }
        return path[num_steps];
    }

}
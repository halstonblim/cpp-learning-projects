#include "mc_option_pricer.h"
#include "welford.h" 
#include <random>

MCOptionPricer::MCResult MCOptionPricer::price_european_call_mc(const MCOptionPricer::MCParams& params) {
    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> normal(0.0, 1.0);
    
    OnlineStats<double> stats;

    for (std::size_t i = 0; i < params.num_paths; ++i) {
        // Sample from N(0,1)
        double Z = normal(gen);
        
        // Calculate the stock price at expiry
        double drift = params.rate - 0.5 * params.vol * params.vol;
        double S_T = params.spot * std::exp(drift * params.expiry + 
                                             params.vol * std::sqrt(params.expiry) * Z);
        
        // Calculate payoff
        double payoff = std::max(S_T - params.strike, 0.0);
        
        // Update stats (you'll need to implement this)
        stats.Add(payoff);
    }

    return {
        std::exp(-params.rate * params.expiry) * stats.Mean(),
        std::exp(-params.rate * params.expiry) * std::sqrt(stats.SampleVariance() / params.num_paths)
    };
}
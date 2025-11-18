#pragma once

#include "stats/OnlineStats.h"
#include "mc/models/IProcessModel.h"
#include <random>

namespace derivlib::mc {

struct MCResult {
    double price;
    double std_error;
};

class MonteCarloEngine {
public:
    MonteCarloEngine(std::unique_ptr<models::IProcessModel> model, std::size_t num_paths, std::size_t num_steps);

    template <typename Payoff>
    MCResult price(const Payoff& payoff, double expiry, double rate);

private:
    std::unique_ptr<models::IProcessModel> model_;
    std::size_t num_paths_;
    std::size_t num_steps_;
    std::mt19937 rng_;
};

template<typename Payoff>
MCResult MonteCarloEngine::price(const Payoff& payoff, double expiry, double rate) {
    OnlineStats<double> payoff_stats;

    std::vector<double> path;
    for (std::size_t i = 0; i < num_paths_; ++i) {
        model_->generate_path(expiry, num_steps_, path, rng_);
        double payoff_val = payoff(path); //functor
        payoff_stats.Add(payoff_val);
    }

    double discount_factor = std::exp(-rate * expiry);
    double mean_payoff = payoff_stats.Mean() * discount_factor;
    double std_err = std::sqrt(payoff_stats.SampleVariance() / num_paths_) * discount_factor;

    return {mean_payoff, std_err};
}

} // namespace derivlib::mc


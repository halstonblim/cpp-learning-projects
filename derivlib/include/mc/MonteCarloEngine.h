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

    template <typename Payoff>
    MCResult price_antithetic(const Payoff& payoff, double expiry, double rate);

private:
    std::unique_ptr<models::IProcessModel> model_;
    std::size_t num_paths_;
    std::size_t num_steps_;
    std::mt19937 rng_;
};

template<typename Payoff>
MCResult MonteCarloEngine::price(const Payoff& payoff, double expiry, double rate) {
    OnlineStats<double> payoff_stats;

    std::normal_distribution<double> normal(0.0, 1.0);
    std::vector<double> path(num_steps_ + 1);
    std::vector<double> variates(num_steps_);

    for (std::size_t i = 0; i < num_paths_; ++i) {
        for(auto& v : variates) {
            v = normal(rng_);
        }
        model_->generate_path(expiry, path, variates);
        double payoff_val = payoff(path); //functor
        payoff_stats.Add(payoff_val);
    }

    double discount_factor = std::exp(-rate * expiry);
    double mean_payoff = payoff_stats.Mean() * discount_factor;
    double std_err = std::sqrt(payoff_stats.SampleVariance() / num_paths_) * discount_factor;

    return {mean_payoff, std_err};
}

template<typename Payoff>
MCResult MonteCarloEngine::price_antithetic(const Payoff& payoff, double expiry, double rate) {
    OnlineStats<double> payoff_stats;

    std::normal_distribution<double> normal(0.0, 1.0);
    std::vector<double> path(num_steps_ + 1);
    std::vector<double> variates(num_steps_);

    for (std::size_t i = 0; i < num_paths_; ++i) {
        for(auto& v : variates) v = normal(rng_);
        model_->generate_path(expiry, path, variates);
        double p1 = payoff(path);

        for(auto& v : variates) v = -v;
        model_->generate_path(expiry, path, variates);
        double p2 = payoff(path);
        payoff_stats.Add(0.5 * (p1 + p2));   
    }

    double discount_factor = std::exp(-rate * expiry);
    double mean_payoff = payoff_stats.Mean() * discount_factor;
    double std_err = std::sqrt(payoff_stats.SampleVariance() / num_paths_) * discount_factor;

    return {mean_payoff, std_err};
}

} // namespace derivlib::mc


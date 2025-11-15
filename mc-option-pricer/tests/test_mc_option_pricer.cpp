#include <gtest/gtest.h>
#include <mc_option_pricer.h>

namespace {

    // Helper: Standard normal CDF
    double norm_cdf(double x) {
        return 0.5 * std::erfc(-x / std::sqrt(2.0));
    }
    
    // Helper: Black-Scholes analytical solution for European call
    double black_scholes_call(double S, double K, double r, 
                              double sigma, double T) {
        double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) 
                    / (sigma * std::sqrt(T));
        double d2 = d1 - sigma * std::sqrt(T);
        return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
    }
    
}

TEST(MCOptionPricerTest, StandardErrorTest) {
    MCOptionPricer::MCParams params{100.0, 120.0, 0.05, 0.2, 1.0, 100000};
    MCOptionPricer::MCResult result = MCOptionPricer::price_european_call_mc( params);

    double analytical = black_scholes_call(params.spot, params.strike, params.rate, params.vol, params.expiry);
    EXPECT_NEAR(result.price, analytical, 5 * result.std_error);
}

TEST(MCOptionPricerTest, ConvergenceTest) {
    MCOptionPricer::MCParams params{100.0, 120.0, 0.05, 0.2, 1.0, 10000000};
    MCOptionPricer::MCResult result = MCOptionPricer::price_european_call_mc( params);

    double analytical = black_scholes_call(params.spot, params.strike, params.rate, params.vol, params.expiry);
    EXPECT_NEAR(result.price, analytical, 0.01);
}

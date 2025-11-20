#include <gtest/gtest.h>
#include "mc/MonteCarloEngine.h"
#include "mc/models/BlackScholesModel.h"
#include "mc/payoffs/EuropeanPayoff.h"

namespace {

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

TEST(MonteCarloTest, BlackScholesCallPrice) {

    double spot = 100.0;
    double strike = 125.0;
    double r = 0.05;
    double vol = 0.2;
    double expiry = 1.0;
    size_t num_steps = 100;
    size_t num_paths = 10000;

    double expected_price = black_scholes_call(spot, strike, r, vol, expiry);

    // setup engine
    auto model = std::make_unique<derivlib::mc::models::BlackScholesModel>(spot, r, vol);
    derivlib::mc::MonteCarloEngine engine(std::move(model), num_paths, num_steps);

    // calculate price
    derivlib::mc::payoffs::EuropeanCall eurocall_payoff(strike);
    derivlib::mc::MCResult result = engine.price(eurocall_payoff, expiry, r);

    // compare
    EXPECT_NEAR(expected_price, result.price, 3.0*result.std_error);

    std::cout << "Standard MC Price: " << result.price << " (Ref: " << expected_price << ")" << std::endl;
    std::cout << "Standard Error: " << result.std_error << std::endl;    
}

TEST(MonteCarloTest, AntitheticVarianceReduction) {

    double spot = 100.0;
    double strike = 125.0;
    double r = 0.05;
    double vol = 0.2;
    double expiry = 1.0;
    size_t num_steps = 100;
    size_t num_paths = 10000;

    double expected_price = black_scholes_call(spot, strike, r, vol, expiry);

    // setup engine
    auto model = std::make_unique<derivlib::mc::models::BlackScholesModel>(spot, r, vol);
    derivlib::mc::MonteCarloEngine engine(std::move(model), num_paths, num_steps);

    // 1. Standard MC (2 * n_pairs)
    auto model_std = std::make_unique<derivlib::mc::models::BlackScholesModel>(spot, r, vol);
    derivlib::mc::MonteCarloEngine engine_std(std::move(model_std), num_paths * 2, num_steps);
    derivlib::mc::payoffs::EuropeanCall payoff(strike);
    auto res_std = engine_std.price(payoff, expiry, r);

    // 2. Antithetic MC (n_pairs)
    auto model_anti = std::make_unique<derivlib::mc::models::BlackScholesModel>(spot, r, vol);
    derivlib::mc::MonteCarloEngine engine_anti(std::move(model_anti), num_paths, num_steps);
    auto res_anti = engine_anti.price_antithetic(payoff, expiry, r);

    std::cout << "Standard MC Error (2N paths): " << res_std.std_error << std::endl;
    std::cout << "Antithetic MC Error (N pairs): " << res_anti.std_error << std::endl;


    // compare
    EXPECT_LT(res_anti.std_error, res_std.std_error);
    EXPECT_NEAR(res_anti.price, res_std.price, 3*(res_anti.std_error + res_std.std_error));

    std::cout << "Antithetic MC Price: " << res_anti.price << " Standard MC Price: " << res_std.price << " (Ref: " << expected_price << ")" << std::endl;
}
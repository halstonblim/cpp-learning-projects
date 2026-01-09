#include <gtest/gtest.h>
#include "core/universe_store.hpp"
#include "compute/signal_engine.hpp"
#include <cmath>

class SignalEngineTest : public ::testing::Test {
protected: 
    static constexpr std::size_t NUM_ASSETS = 16;
    UniverseStore store{NUM_ASSETS};
    SignalEngine engine{store};

    void SetUp() override {
        store.write_lock();
        for (size_t i = 0; i < NUM_ASSETS; ++i) {
            store.update_tick({
                static_cast<uint32_t>(i),
                static_cast<float>(i),
                100.0f, 0.0f, 0.0f
            });
        }
        store.write_unlock();
    }
};

TEST_F(SignalEngineTest, MeanCalculationIsCorrect) {
    float expected_mean{};
    for(size_t i=0; i<NUM_ASSETS; ++i) {
        expected_mean +=  static_cast<float>(i);
    }    
    expected_mean = expected_mean / NUM_ASSETS;
    float actual_mean = engine.calculate_mean_avx();
    EXPECT_FLOAT_EQ(actual_mean, expected_mean);
}

TEST_F(SignalEngineTest, StatsCalculationIsCorrect) {
    float expected_mean{};
    for(size_t i=0; i<NUM_ASSETS; ++i) {
        expected_mean +=  static_cast<float>(i);
    }        
    expected_mean = expected_mean / NUM_ASSETS;
    
    float sum_sq_diff{};
    for(size_t i=0; i<NUM_ASSETS; ++i) {
        float diff = static_cast<float>(i) - expected_mean;
        sum_sq_diff += diff * diff;
    }
    float expected_std_dev = std::sqrt(sum_sq_diff / NUM_ASSETS);

    PriceStats stats = engine.calculate_stats_avx();

    EXPECT_FLOAT_EQ(stats.mean, expected_mean);
    EXPECT_NEAR(stats.std_dev, expected_std_dev, 1e-5);
}

TEST_F(SignalEngineTest, ZScoreIsCorrect) {
    float expected_mean{};
    for(size_t i=0; i<NUM_ASSETS; ++i) {
        expected_mean +=  static_cast<float>(i);
    }        
    expected_mean = expected_mean / NUM_ASSETS;
    
    float sum_sq_diff{};
    for(size_t i=0; i<NUM_ASSETS; ++i) {
        float diff = static_cast<float>(i) - expected_mean;
        sum_sq_diff += diff * diff;
    }
    float expected_std_dev = std::sqrt(sum_sq_diff / NUM_ASSETS);

    float expected_z_scores[NUM_ASSETS];

    for(size_t i=0; i<NUM_ASSETS; ++i) {
        expected_z_scores[i] = (static_cast<float>(i) - expected_mean) / expected_std_dev;
    }    

    float actual_z_scores[NUM_ASSETS];
    engine.calculate_zscores_avx(actual_z_scores);

    for (size_t i=0; i < NUM_ASSETS; ++i) {
        EXPECT_FLOAT_EQ(actual_z_scores[i], expected_z_scores[i]) << "Mismatch at index " << i;
    }
}

#include <gtest/gtest.h>
#include "time_series.hpp"

class SIMDTest : public ::testing::Test {
protected:
    TimeSeries timeseries{10};
};

TEST_F(SIMDTest, CompareScalarAndSimd) {
    for(int i=0; i<10; ++i) timeseries.add_tick(static_cast<double>(i));
    
    EXPECT_DOUBLE_EQ(timeseries.get_mean(), timeseries.get_mean_simd());
}

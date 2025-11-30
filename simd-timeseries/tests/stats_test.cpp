#include <gtest/gtest.h>
#include "time_series.hpp"

class TimeSeriesTest : public ::testing::Test {
protected:
    TimeSeries timeseries{3};
};

TEST_F(TimeSeriesTest, CalculatesBasicMean) {
    timeseries.add_tick(1.0);
    timeseries.add_tick(2.0);
    timeseries.add_tick(3.0);

    EXPECT_DOUBLE_EQ(timeseries.get_mean(), 2.0);
}

TEST_F(TimeSeriesTest, HandlesWrapAround) {
    timeseries.add_tick(1.0);
    timeseries.add_tick(2.0);
    timeseries.add_tick(3.0);
    
    timeseries.add_tick(4.0); 

    // After wrap-around: array is {4.0, 2.0, 3.0}, sum = 9.0, mean = 3.0
    EXPECT_DOUBLE_EQ(timeseries.get_mean(), 3.0);
}
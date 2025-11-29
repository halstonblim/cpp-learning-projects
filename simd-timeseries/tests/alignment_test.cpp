#include <gtest/gtest.h>
#include "time_series.hpp"

class AlignedAllocatorTest : public ::testing::Test {
protected:
    TimeSeries timeseries{3};
};

TEST_F(AlignedAllocatorTest, CalculatesBasicMean) {
    timeseries.add_tick(1.0);
    timeseries.add_tick(2.0);
    timeseries.add_tick(3.0);

    auto ptr_address = timeseries.get_data();
    auto ptr_value = reinterpret_cast<std::uintptr_t>(ptr_address);

    EXPECT_EQ(ptr_value % 32, 0);
}

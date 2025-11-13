#include <gtest/gtest.h>
#include <welford.h>
#include <cmath>
#include <vector>

// Test fixture for OnlineStats tests
template <typename RealType>
class OnlineStatsTest : public ::testing::Test {
protected:
    OnlineStats<RealType> stats;
    
    // Helper function to add multiple values
    void AddValues(const std::vector<RealType>& values) {
        for (const auto& value : values) {
            stats.Add(value);
        }
    }
    
    // Helper function to check if two floating point numbers are approximately equal
    bool IsApproxEqual(RealType a, RealType b, RealType tolerance = 1e-5) {
        return std::abs(a - b) < tolerance;
    }
};

// Typed test suite for different numeric types
using NumericTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(OnlineStatsTest, NumericTypes);

// Test: Initial state
TYPED_TEST(OnlineStatsTest, InitialState) {
    EXPECT_EQ(this->stats.Count(), 0);
    EXPECT_EQ(this->stats.Mean(), 0);
    EXPECT_EQ(this->stats.Variance(), 0);
}

// Test: Adding a single value
TYPED_TEST(OnlineStatsTest, SingleValue) {
    TypeParam value = 5.0;
    this->stats.Add(value);
    
    EXPECT_EQ(this->stats.Count(), 1);
    EXPECT_EQ(this->stats.Mean(), value);
    EXPECT_EQ(this->stats.Variance(), 0);  // Variance of single value is 0
}

// Test: Adding multiple values
TYPED_TEST(OnlineStatsTest, MultipleValues) {
    std::vector<TypeParam> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    this->AddValues(values);
    
    EXPECT_EQ(this->stats.Count(), 5);
    EXPECT_EQ(this->stats.Mean(), 3.0);  // (1+2+3+4+5)/5 = 3.0
    
    // Variance calculation: sum((x - mean)^2) / n
    // (1-3)^2 + (2-3)^2 + (3-3)^2 + (4-3)^2 + (5-3)^2 = 4+1+0+1+4 = 10
    // Variance = 10/5 = 2.0
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Variance(), 2.0));
}

// Test: Mean calculation accuracy
TYPED_TEST(OnlineStatsTest, MeanCalculation) {
    std::vector<TypeParam> values = {10.5, 20.3, 30.7, 40.1, 50.9};
    this->AddValues(values);
    
    TypeParam expected_mean = (10.5 + 20.3 + 30.7 + 40.1 + 50.9) / 5.0;
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Mean(), expected_mean));
}

// Test: Variance calculation accuracy
TYPED_TEST(OnlineStatsTest, VarianceCalculation) {
    std::vector<TypeParam> values = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    this->AddValues(values);
    
    // Manual calculation:
    // Mean = (2+4+4+4+5+5+7+9)/8 = 40/8 = 5.0
    // Variance = sum((x-5)^2)/8 = (9+1+1+1+0+0+4+16)/8 = 32/8 = 4.0
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Mean(), 5.0));
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Variance(), 4.0));
}

// Test: Large number of values
TYPED_TEST(OnlineStatsTest, LargeDataSet) {
    const int num_values = 1000;
    TypeParam sum = 0;
    
    for (int i = 1; i <= num_values; ++i) {
        TypeParam value = static_cast<TypeParam>(i);
        this->stats.Add(value);
        sum += value;
    }
    
    TypeParam expected_mean = sum / num_values;
    EXPECT_EQ(this->stats.Count(), num_values);
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Mean(), expected_mean));
    
    // Variance of 1..n is approximately n^2/12 for large n
    // More precisely: sum(i^2)/n - mean^2 where mean = (n+1)/2
    TypeParam expected_variance = (num_values * (num_values + 1) * (2 * num_values + 1)) / (6.0 * num_values) - 
                                  ((num_values + 1) / 2.0) * ((num_values + 1) / 2.0);
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Variance(), expected_variance, 1e-2));
}

// Test: Incremental updates maintain correctness
TYPED_TEST(OnlineStatsTest, IncrementalUpdates) {
    // Add values one at a time and verify intermediate results
    this->stats.Add(10.0);
    EXPECT_EQ(this->stats.Count(), 1);
    EXPECT_EQ(this->stats.Mean(), 10.0);
    
    this->stats.Add(20.0);
    EXPECT_EQ(this->stats.Count(), 2);
    EXPECT_EQ(this->stats.Mean(), 15.0);
    
    this->stats.Add(30.0);
    EXPECT_EQ(this->stats.Count(), 3);
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Mean(), 20.0));
    
    // Variance after 3 values: ((10-20)^2 + (20-20)^2 + (30-20)^2) / 3 = (100+0+100)/3 = 200/3
    EXPECT_TRUE(this->IsApproxEqual(this->stats.Variance(), 200.0 / 3.0));
}

// Test: Const correctness - methods should be callable on const objects
TYPED_TEST(OnlineStatsTest, ConstCorrectness) {
    this->stats.Add(5.0);
    this->stats.Add(10.0);
    
    const OnlineStats<TypeParam>& const_stats = this->stats;
    
    // All getter methods should be callable on const objects
    EXPECT_EQ(const_stats.Count(), 2);
    EXPECT_EQ(const_stats.Mean(), 7.5);
    EXPECT_GE(const_stats.Variance(), 0);  // Variance should be non-negative
}





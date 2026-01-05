#include <gtest/gtest.h>
#include "core/universe_store.hpp"
#include "compute/signal_engine.hpp"
#include <thread>
#include <atomic>
#include <cstdint>

class TestableSignalEngine : public SignalEngine {
public:
    using SignalEngine::SignalEngine;  // Inherit constructor
    
    // Expose snapshot for testing
    bool verify_snapshot_consistency() {
        snapshot();  // protected and accessible
        for (size_t i = 0; i < snapshot_size_; ++i) {
            if (snapshot_prices_[i] != snapshot_volumes_[i]) {
                return false;  // Torn read detected!
            }
        }
        return true;
    }
};

class SignalEngineSnapshotTest : public ::testing::Test {
protected: 
    static constexpr std::size_t NUM_ASSETS = 16;
    UniverseStore store{NUM_ASSETS};
    TestableSignalEngine engine{store};
};

TEST_F(SignalEngineSnapshotTest, StressTestSnapshotConsistency) {
    static constexpr int NUM_ITERATIONS = 100'000;
    std::atomic<bool> writer_done{false};
    std::atomic<bool> torn_read_detected{false};

    // Writer thread: spams updates with price = volume = tick
    std::jthread writer([&] {
        for (uint32_t tick = 0; tick < NUM_ITERATIONS; ++tick) {
            float tick_value = static_cast<float>(tick);
            
            store.write_lock();
            for (uint32_t asset_id = 0; asset_id < NUM_ASSETS; ++asset_id) {
                store.update_tick({asset_id, tick_value, tick_value, 0.0f, 0.0f});
            }
            store.write_unlock();
        }
        writer_done = true;
    });

    // Reader thread: verifies snapshot consistency
    std::jthread reader([&] {
        while (!writer_done) {
            if (!engine.verify_snapshot_consistency()) {
                torn_read_detected = true;
                break;
            }
        }
    });

    // jthreads auto-join when they go out of scope

    EXPECT_FALSE(torn_read_detected) << "Torn read detected! Snapshot is not atomic.";
}

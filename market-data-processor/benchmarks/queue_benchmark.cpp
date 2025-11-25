#include <chrono>
#include <format>
#include <iostream>
#include <optional>
#include <string_view>
#include <thread>

#include "lock_based_queue.hpp"
#include "lock_free_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"
#include "types.hpp"

// End-to-end throughput and latency benchmark for queue implementations.
// Runs producer/consumer simulation with 1M ticks and exports latency data.

namespace {

constexpr int NUM_TICKS = 1'000'000;
constexpr size_t RING_BUFFER_SIZE = 1024;

// Queue returning std::optional<T> from pop()
template <typename Q, typename T>
concept OptionalPopQueue = requires(Q q, const T& item) {
    { q.push(item) } -> std::same_as<bool>;
    { q.pop() } -> std::same_as<std::optional<T>>;
};

// Queue using try_pop(T&) -> bool
template <typename Q, typename T>
concept TryPopQueue = requires(Q q, T& out, const T& item) {
    { q.push(item) } -> std::same_as<void>;
    { q.try_pop(out) } -> std::same_as<bool>;
};

// Adapts both pop interfaces to optional<T>
template <typename Q, typename T>
[[nodiscard]] std::optional<T> try_pop_unified(Q& queue) {
    if constexpr (OptionalPopQueue<Q, T>) {
        return queue.pop();
    } else {
        T value;
        if (queue.try_pop(value)) {
            return value;
        }
        return std::nullopt;
    }
}

// Spin-retry for lock-free, blocking for lock-based
template <typename Q, typename T>
void push_unified(Q& queue, const T& item) {
    if constexpr (OptionalPopQueue<Q, T>) {
        while (!queue.push(item)) {
            std::this_thread::yield();
        }
    } else {
        queue.push(item);
    }
}

// Constructs queue with size param if required
template <typename Q>
[[nodiscard]] auto make_queue() {
    if constexpr (requires { Q(size_t{}); }) {
        return Q(RING_BUFFER_SIZE);
    } else {
        return Q{};
    }
}

// RAII timer: records duration on destruction
class ScopedTimer {
public:
    explicit ScopedTimer(std::chrono::milliseconds& out)
        : start_{std::chrono::steady_clock::now()}, output_{out} {}

    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        output_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    std::chrono::steady_clock::time_point start_;
    std::chrono::milliseconds& output_;
};

template <typename QueueType>
void run_simulation(std::string_view name, const std::string& csv_filename = "") {
    std::cout << std::format("Starting Benchmark: {} ({} ticks)...\n", name, NUM_TICKS);

    auto queue = make_queue<QueueType>();
    MarketSimulator sim;
    SignalEngine engine;

    std::chrono::milliseconds duration{};

    {
        ScopedTimer timer{duration};

        std::jthread producer{[&] {
            for (int i = 0; i < NUM_TICKS; ++i) {
                push_unified<QueueType, Tick>(queue, sim.next_tick());
            }
        }};

        std::jthread consumer{[&] {
            for (int processed = 0; processed < NUM_TICKS;) {
                if (auto tick = try_pop_unified<QueueType, Tick>(queue)) {
                    engine.process_tick(*tick);
                    ++processed;
                } else {
                    std::this_thread::yield();
                }
            }
        }};
    }  // jthreads join here, timer records duration

    double seconds = duration.count() / 1000.0;
    std::cout << std::format("Total Wall Time: {}ms\n", duration.count());
    std::cout << std::format("Throughput: {:.0f} ticks/sec\n", NUM_TICKS / seconds);

    engine.write_latency_report();

    if (!csv_filename.empty()) {
        engine.export_latencies_csv(csv_filename);
    }

    std::cout << std::string(50, '-') << "\n\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string_view mode = (argc > 1) ? argv[1] : "both";

    if (mode == "lock" || mode == "both") {
        run_simulation<ThreadSafeQueue<Tick>>("Lock-Based (Mutex)", "data/latency_lock_based.csv");
    }

    if (mode == "free" || mode == "both") {
        run_simulation<LockFreeQueue<Tick>>("Lock-Free (Atomic)", "data/latency_lock_free.csv");
    }

    if (mode == "both") {
        std::cout << "\n=== CSV files exported for visualization ===" << std::endl;
        std::cout << "  - data/latency_lock_based.csv" << std::endl;
        std::cout << "  - data/latency_lock_free.csv" << std::endl;
        std::cout << "\nRun: python3 scripts/visualize_latency.py" << std::endl;
    }
}

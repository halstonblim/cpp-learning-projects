#include <vector>

class TimeSeries {
public:
    TimeSeries(size_t max_capacity) : head_(0), is_full_(false), capacity_(max_capacity) {
        data.resize(max_capacity);
    }
    void add_tick(double price);
    [[nodiscard]]double get_mean() const;
    [[nodiscard]]size_t size() const;
    [[nodiscard]] size_t capacity() const;    
    void clear();

private:
    size_t head_;
    std::vector<double> data;
    size_t capacity_;
    bool is_full_;
};
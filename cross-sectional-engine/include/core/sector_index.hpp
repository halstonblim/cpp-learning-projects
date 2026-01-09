#pragma once
#include <vector>
#include <cstdint>

class SectorIndex {
public:
    SectorIndex(size_t num_assets, size_t num_sectors,
        const std::vector<uint32_t>& sector_assignments);
    
    [[nodiscard]] size_t sector_start(uint32_t sector_id) const;
    [[nodiscard]] size_t sector_end(uint32_t sector_id) const;
    [[nodiscard]] size_t num_sectors() const;
    [[nodiscard]] size_t sorted_index(uint32_t asset_id) const;

private:
    size_t num_sectors_;
    std::vector<size_t> sector_offsets_;
    std::vector<size_t> asset_to_sorted_;
};
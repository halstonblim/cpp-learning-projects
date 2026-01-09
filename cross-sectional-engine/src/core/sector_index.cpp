#include "core/sector_index.hpp"

SectorIndex::SectorIndex(size_t num_assets, size_t num_sectors, 
                         const std::vector<uint32_t>& sector_assignments) 
: num_sectors_(num_sectors),
  sector_offsets_(num_sectors+1,0), 
  asset_to_sorted_(num_assets)
{
    for (uint32_t sector_id : sector_assignments) ++sector_offsets_[sector_id + 1];
    for (size_t i = 1; i <= num_sectors; ++i) sector_offsets_[i] += sector_offsets_[i - 1];

    std::vector<size_t> write_position = sector_offsets_;
    for (size_t i = 0; i < num_assets; ++i) {
        asset_to_sorted_[i] = write_position[sector_assignments[i]]++;
    }
}

size_t SectorIndex::sector_start(uint32_t sector_id) const {
    return sector_offsets_[sector_id];
}

size_t SectorIndex::sector_end(uint32_t sector_id) const {
    return sector_offsets_[sector_id + 1];
}

size_t SectorIndex::num_sectors() const {
    return num_sectors_;
}

size_t SectorIndex::sorted_index(uint32_t asset_id) const {
    return asset_to_sorted_[asset_id];
}
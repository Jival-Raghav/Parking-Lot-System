#pragma once

#include "types.h"
#include <vector>
#include <cstdint>

namespace parking::slots {

// Identifies all usable 'P' slots in the grid, calculates their distances,
// sorts them by (distance, row, col), and builds:
// - usable_slots list
// - rank_to_cell map (rank 0..k-1 -> flat cell index)
// - cell_to_rank map (flat cell index -> rank 0..k-1, or -1 if unusable/not a slot)
// - populates free_ranks priority queue with all ranks 0..k-1
void build_usable_slots(
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    const std::vector<int32_t>& road_dist,
    LotState& lot
);

} // namespace parking::slots

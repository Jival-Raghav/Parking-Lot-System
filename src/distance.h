#pragma once

#include "types.h"
#include <vector>
#include <cstdint>

namespace parking::distance {

// Computes shortest distance from entrance 'E' to all reachable road cells ('.' and 'E')
// using one iterative BFS (no recursion).
// queue is preallocated as std::vector<int32_t> with a head index.
// Returns a vector of distances of size (rows * cols), where -1 indicates unreachable.
std::vector<int32_t> compute_road_distances(
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    int32_t entrance_idx
);

// Computes the distance for each slot cell 'P' in the grid.
// A slot can only be parked in if it is directly adjacent (up/down/left/right)
// to a reachable road cell or 'E'.
// Distance = min(reachable neighbour road distance) + 1.
// If no neighbour is a reachable road cell, the slot is unusable (distance = -1).
// Cars cannot drive through other 'P' cells or walls '#'.
int32_t compute_slot_distance(
    int32_t slot_cell_index,
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    const std::vector<int32_t>& road_dist
);

} // namespace parking::distance

#include "slots.h"
#include "distance.h"
#include "grid.h"
#include <algorithm>

namespace parking::slots {

void build_usable_slots(
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    const std::vector<int32_t>& road_dist,
    LotState& lot
) {
    size_t total_cells = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    lot.cell_to_rank.assign(total_cells, -1);
    lot.usable_slots.clear();
    lot.rank_to_cell.clear();
    
    // Find all 'P' cells and compute distance
    for (size_t i = 0; i < total_cells; ++i) {
        if (grid[i] == 'P') {
            int32_t d = distance::compute_slot_distance(static_cast<int32_t>(i), rows, cols, grid, road_dist);
            if (d != -1) {
                Position pos = grid::to_position(i, cols);
                lot.usable_slots.push_back(UsableSlot{d, pos.row, pos.col, static_cast<int32_t>(i)});
            }
        }
    }
    
    // Sort usable slots by: distance ASC, row ASC, col ASC
    std::sort(lot.usable_slots.begin(), lot.usable_slots.end(), [](const UsableSlot& a, const UsableSlot& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });
    
    lot.usable_count = static_cast<int32_t>(lot.usable_slots.size());
    lot.rank_to_cell.resize(lot.usable_slots.size());
    
    // Clear and initialize free_ranks heap
    while (!lot.free_ranks.empty()) {
        lot.free_ranks.pop();
    }
    
    for (size_t rank = 0; rank < lot.usable_slots.size(); ++rank) {
        int32_t cell_idx = lot.usable_slots[rank].cell_index;
        lot.rank_to_cell[rank] = cell_idx;
        lot.cell_to_rank[static_cast<size_t>(cell_idx)] = static_cast<int32_t>(rank);
        lot.free_ranks.push(static_cast<int32_t>(rank));
    }
    
    // Initialize occupancy
    lot.rank_occupant.assign(lot.usable_slots.size(), std::nullopt);
    lot.reg_to_cell.clear();
    lot.colour_to_cells.clear();
    lot.all_occupied.clear();
}

} // namespace parking::slots

#include "state.h"
#include "grid.h"

namespace parking::state {

bool is_registration_parked(const LotState& lot, const std::string& reg) {
    return lot.reg_to_cell.find(reg) != lot.reg_to_cell.end();
}

bool is_usable_slot(const LotState& lot, int32_t cell_index) {
    if (cell_index < 0 || static_cast<size_t>(cell_index) >= lot.cell_to_rank.size()) {
        return false;
    }
    return lot.cell_to_rank[static_cast<size_t>(cell_index)] != -1;
}

bool is_slot_occupied(const LotState& lot, int32_t cell_index) {
    if (!is_usable_slot(lot, cell_index)) {
        return false;
    }
    int32_t rank = lot.cell_to_rank[static_cast<size_t>(cell_index)];
    return lot.rank_occupant[static_cast<size_t>(rank)].has_value();
}

Position park_car(LotState& lot, const std::string& reg, const std::string& colour) {
    int32_t rank = lot.free_ranks.top();
    lot.free_ranks.pop();
    
    int32_t cell_idx = lot.rank_to_cell[static_cast<size_t>(rank)];
    lot.rank_occupant[static_cast<size_t>(rank)] = Car{reg, colour};
    
    lot.reg_to_cell[reg] = cell_idx;
    lot.colour_to_cells[to_lower_ascii(colour)].insert(cell_idx);
    lot.all_occupied.insert(cell_idx);
    
    return grid::to_position(static_cast<size_t>(cell_idx), lot.cols);
}

void free_slot(LotState& lot, int32_t cell_index) {
    int32_t rank = lot.cell_to_rank[static_cast<size_t>(cell_index)];
    const Car& car = *(lot.rank_occupant[static_cast<size_t>(rank)]);
    
    std::string colour_lower = to_lower_ascii(car.colour);
    
    // Remove from indexes
    lot.reg_to_cell.erase(car.registration);
    
    auto it = lot.colour_to_cells.find(colour_lower);
    if (it != lot.colour_to_cells.end()) {
        it->second.erase(cell_index);
        if (it->second.empty()) {
            lot.colour_to_cells.erase(it);
        }
    }
    
    lot.all_occupied.erase(cell_index);
    lot.rank_occupant[static_cast<size_t>(rank)] = std::nullopt;
    
    // Push rank back into min-heap
    lot.free_ranks.push(rank);
}

} // namespace parking::state

#pragma once

#include "types.h"
#include <string>
#include <string_view>
#include <cctype>

namespace parking::state {

// ASCII case-insensitive helper
inline std::string to_lower_ascii(std::string_view str) {
    std::string res;
    res.reserve(str.size());
    for (char ch : str) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return res;
}

// Check if registration is already parked
bool is_registration_parked(const LotState& lot, const std::string& reg);

// Check if a cell is an in-bounds usable parking slot
bool is_usable_slot(const LotState& lot, int32_t cell_index);

// Check if a usable slot is currently occupied
bool is_slot_occupied(const LotState& lot, int32_t cell_index);

// Park a car into the best available free slot.
// Returns the allocated Position.
// Preconditions: lot has free slots, reg is not already parked.
Position park_car(LotState& lot, const std::string& reg, const std::string& colour);

// Free an occupied slot.
// Returns true on success.
// Precondition: cell_index is an occupied usable slot.
void free_slot(LotState& lot, int32_t cell_index);

} // namespace parking::state

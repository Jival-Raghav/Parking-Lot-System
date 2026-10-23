#pragma once

#include "types.h"
#include <cstdint>
#include <vector>
#include <string>

namespace parking::grid {

// Coordinate conversion helpers
inline size_t to_cell_index(int32_t row_1based, int32_t col_1based, int32_t cols) {
    return static_cast<size_t>(row_1based - 1) * static_cast<size_t>(cols) + static_cast<size_t>(col_1based - 1);
}

inline Position to_position(size_t cell_index, int32_t cols) {
    int32_t r = static_cast<int32_t>(cell_index / static_cast<size_t>(cols)) + 1;
    int32_t c = static_cast<int32_t>(cell_index % static_cast<size_t>(cols)) + 1;
    return Position{r, c};
}

inline bool is_valid_coordinate(int32_t row, int32_t col, int32_t total_rows, int32_t total_cols) {
    return row >= 1 && row <= total_rows && col >= 1 && col <= total_cols;
}

// 4-directional neighbours (up, down, left, right)
// Returns list of valid 0-based cell indices
std::vector<int32_t> get_neighbours(int32_t cell_index, int32_t rows, int32_t cols);

// Validate map characters and dimensions.
// Returns true if valid, false otherwise.
// If valid, also returns the index of the entrance 'E'.
bool validate_map(int32_t rows, int32_t cols, const std::vector<std::string>& map_lines, int32_t& out_entrance_idx);

} // namespace parking::grid

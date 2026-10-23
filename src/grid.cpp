#include "grid.h"

namespace parking::grid {

std::vector<int32_t> get_neighbours(int32_t cell_index, int32_t rows, int32_t cols) {
    std::vector<int32_t> neighbours;
    neighbours.reserve(4);
    
    int32_t r = static_cast<int32_t>(cell_index / cols); // 0-based
    int32_t c = static_cast<int32_t>(cell_index % cols); // 0-based
    
    // Up
    if (r > 0) {
        neighbours.push_back((r - 1) * cols + c);
    }
    // Down
    if (r + 1 < rows) {
        neighbours.push_back((r + 1) * cols + c);
    }
    // Left
    if (c > 0) {
        neighbours.push_back(r * cols + (c - 1));
    }
    // Right
    if (c + 1 < cols) {
        neighbours.push_back(r * cols + (c + 1));
    }
    
    return neighbours;
}

bool validate_map(int32_t rows, int32_t cols, const std::vector<std::string>& map_lines, int32_t& out_entrance_idx) {
    if (static_cast<int32_t>(map_lines.size()) != rows) {
        return false;
    }
    
    int entrance_count = 0;
    out_entrance_idx = -1;
    
    for (int32_t r = 0; r < rows; ++r) {
        const std::string& line = map_lines[static_cast<size_t>(r)];
        if (static_cast<int32_t>(line.size()) != cols) {
            return false;
        }
        for (int32_t c = 0; c < cols; ++c) {
            char ch = line[static_cast<size_t>(c)];
            if (ch == 'E') {
                entrance_count++;
                out_entrance_idx = r * cols + c;
            } else if (ch != '.' && ch != '#' && ch != 'P') {
                return false;
            }
        }
    }
    
    return (entrance_count == 1);
}

} // namespace parking::grid

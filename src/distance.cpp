#include "distance.h"
#include "grid.h"
#include <algorithm>
#include <limits>

namespace parking::distance {

std::vector<int32_t> compute_road_distances(
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    int32_t entrance_idx
) {
    size_t total = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    std::vector<int32_t> dist(total, -1);
    
    if (entrance_idx < 0 || static_cast<size_t>(entrance_idx) >= total) {
        return dist;
    }
    
    // Preallocated BFS queue with head pointer
    std::vector<int32_t> q;
    q.reserve(total);
    
    dist[static_cast<size_t>(entrance_idx)] = 0;
    q.push_back(entrance_idx);
    size_t head = 0;
    
    while (head < q.size()) {
        int32_t curr = q[head++];
        int32_t curr_d = dist[static_cast<size_t>(curr)];
        
        std::vector<int32_t> neighbours = grid::get_neighbours(curr, rows, cols);
        for (int32_t next : neighbours) {
            char cell = grid[static_cast<size_t>(next)];
            // Cars drive ONLY on road cells ('.' and 'E')
            if ((cell == '.' || cell == 'E') && dist[static_cast<size_t>(next)] == -1) {
                dist[static_cast<size_t>(next)] = curr_d + 1;
                q.push_back(next);
            }
        }
    }
    
    return dist;
}

int32_t compute_slot_distance(
    int32_t slot_cell_index,
    int32_t rows,
    int32_t cols,
    const std::vector<char>& grid,
    const std::vector<int32_t>& road_dist
) {
    (void)grid;
    std::vector<int32_t> neighbours = grid::get_neighbours(slot_cell_index, rows, cols);
    int32_t min_neighbour_dist = std::numeric_limits<int32_t>::max();
    
    for (int32_t n : neighbours) {
        int32_t d = road_dist[static_cast<size_t>(n)];
        if (d != -1 && d < min_neighbour_dist) {
            min_neighbour_dist = d;
        }
    }
    
    if (min_neighbour_dist == std::numeric_limits<int32_t>::max()) {
        return -1; // Unusable
    }
    
    return min_neighbour_dist + 1;
}

} // namespace parking::distance

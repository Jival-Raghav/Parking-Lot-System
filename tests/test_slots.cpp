#include "test_framework.h"
#include "../src/slots.h"
#include "../src/distance.h"
#include "../src/grid.h"

using namespace parking;

TEST_CASE(test_sample_lot_slots_ordering) {
    int32_t rows = 5;
    int32_t cols = 7;
    std::vector<std::string> map_lines = {
        "E.....P",
        "P#P#P.P",
        "P.....#",
        "PP#P#.P",
        "P####.P"
    };

    int32_t entrance_idx = -1;
    grid::validate_map(rows, cols, map_lines, entrance_idx);
    std::vector<char> grid_data(static_cast<size_t>(rows * cols));
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            grid_data[static_cast<size_t>(r * cols + c)] = map_lines[static_cast<size_t>(r)][static_cast<size_t>(c)];
        }
    }

    auto road_dist = distance::compute_road_distances(rows, cols, grid_data, entrance_idx);
    LotState lot;
    lot.rows = rows;
    lot.cols = cols;
    lot.grid = grid_data;

    slots::build_usable_slots(rows, cols, grid_data, road_dist, lot);

    CHECK_EQ(lot.usable_count, 10);
    CHECK_EQ(lot.usable_slots.size(), static_cast<size_t>(10));

    // Expected order of slots:
    // (2, 1), (2, 3), (2, 5), (1, 7), (2, 7), (4, 7), (4, 4), (5, 7), (3, 1), (4, 2)
    std::vector<Position> expected_slots = {
        {2, 1}, {2, 3}, {2, 5}, {1, 7}, {2, 7},
        {4, 7}, {4, 4}, {5, 7}, {3, 1}, {4, 2}
    };

    for (size_t i = 0; i < expected_slots.size(); ++i) {
        CHECK_EQ(lot.usable_slots[i].row, expected_slots[i].row);
        CHECK_EQ(lot.usable_slots[i].col, expected_slots[i].col);
    }

    // Verify cell_to_rank and rank_to_cell
    for (size_t rank = 0; rank < lot.usable_slots.size(); ++rank) {
        int32_t cell = lot.rank_to_cell[rank];
        CHECK_EQ(lot.cell_to_rank[static_cast<size_t>(cell)], static_cast<int32_t>(rank));
    }

    // Verify unusable slots (4, 1) and (5, 1) have rank -1
    int32_t cell_4_1 = 3 * cols + 0;
    int32_t cell_5_1 = 4 * cols + 0;
    CHECK_EQ(lot.cell_to_rank[static_cast<size_t>(cell_4_1)], -1);
    CHECK_EQ(lot.cell_to_rank[static_cast<size_t>(cell_5_1)], -1);
}

int main() {
    return test_framework::run_all_tests();
}

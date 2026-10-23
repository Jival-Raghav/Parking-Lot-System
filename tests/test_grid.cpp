#include "test_framework.h"
#include "../src/grid.h"

using namespace parking;

TEST_CASE(test_coordinate_conversions) {
    // 5 rows, 7 cols
    int32_t cols = 7;
    CHECK_EQ(grid::to_cell_index(1, 1, cols), static_cast<size_t>(0));
    CHECK_EQ(grid::to_cell_index(1, 7, cols), static_cast<size_t>(6));
    CHECK_EQ(grid::to_cell_index(2, 1, cols), static_cast<size_t>(7));
    CHECK_EQ(grid::to_cell_index(5, 7, cols), static_cast<size_t>(34));

    Position p1 = grid::to_position(0, cols);
    CHECK_EQ(p1.row, 1);
    CHECK_EQ(p1.col, 1);

    Position p2 = grid::to_position(6, cols);
    CHECK_EQ(p2.row, 1);
    CHECK_EQ(p2.col, 7);

    Position p3 = grid::to_position(34, cols);
    CHECK_EQ(p3.row, 5);
    CHECK_EQ(p3.col, 7);
}

TEST_CASE(test_valid_coordinate) {
    CHECK(grid::is_valid_coordinate(1, 1, 5, 7));
    CHECK(grid::is_valid_coordinate(5, 7, 5, 7));
    CHECK(!grid::is_valid_coordinate(0, 1, 5, 7));
    CHECK(!grid::is_valid_coordinate(1, 0, 5, 7));
    CHECK(!grid::is_valid_coordinate(6, 7, 5, 7));
    CHECK(!grid::is_valid_coordinate(5, 8, 5, 7));
    CHECK(!grid::is_valid_coordinate(-1, 2, 5, 7));
}

TEST_CASE(test_neighbours_corners_and_middle) {
    int32_t rows = 5;
    int32_t cols = 7;

    // Top-left corner (1, 1) -> index 0: down (8) and right (1)
    auto n_top_left = grid::get_neighbours(0, rows, cols);
    CHECK_EQ(n_top_left.size(), static_cast<size_t>(2));
    CHECK_EQ(n_top_left[0], 7); // down
    CHECK_EQ(n_top_left[1], 1); // right

    // Middle cell (3, 3) -> index 2*7 + 2 = 16
    auto n_mid = grid::get_neighbours(16, rows, cols);
    CHECK_EQ(n_mid.size(), static_cast<size_t>(4));
    CHECK_EQ(n_mid[0], 9);  // up (2, 3)
    CHECK_EQ(n_mid[1], 23); // down (4, 3)
    CHECK_EQ(n_mid[2], 15); // left (3, 2)
    CHECK_EQ(n_mid[3], 17); // right (3, 4)

    // 1x1 grid: cell 0 has 0 neighbours
    auto n_1x1 = grid::get_neighbours(0, 1, 1);
    CHECK_EQ(n_1x1.size(), static_cast<size_t>(0));

    // 1x3 grid: cell 1 has 2 neighbours (left 0, right 2)
    auto n_1x3 = grid::get_neighbours(1, 1, 3);
    CHECK_EQ(n_1x3.size(), static_cast<size_t>(2));
    CHECK_EQ(n_1x3[0], 0);
    CHECK_EQ(n_1x3[1], 2);
}

TEST_CASE(test_validate_map) {
    int32_t entrance_idx = -1;
    std::vector<std::string> valid_map = {
        "E.....P",
        "P#P#P.P",
        "P.....#",
        "PP#P#.P",
        "P####.P"
    };
    CHECK(grid::validate_map(5, 7, valid_map, entrance_idx));
    CHECK_EQ(entrance_idx, 0);

    // Mismatched row count
    CHECK(!grid::validate_map(4, 7, valid_map, entrance_idx));

    // Mismatched col count
    std::vector<std::string> bad_cols = valid_map;
    bad_cols[1] = "P#P#P."; // 6 chars instead of 7
    CHECK(!grid::validate_map(5, 7, bad_cols, entrance_idx));

    // Illegal character
    std::vector<std::string> bad_char = valid_map;
    bad_char[2] = "P..x..#";
    CHECK(!grid::validate_map(5, 7, bad_char, entrance_idx));

    // Zero 'E'
    std::vector<std::string> no_e = valid_map;
    no_e[0] = "......P";
    CHECK(!grid::validate_map(5, 7, no_e, entrance_idx));

    // Multiple 'E'
    std::vector<std::string> multi_e = valid_map;
    multi_e[2] = "P..E..#";
    CHECK(!grid::validate_map(5, 7, multi_e, entrance_idx));
}

int main() {
    return test_framework::run_all_tests();
}

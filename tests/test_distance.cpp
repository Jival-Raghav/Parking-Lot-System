#include "test_framework.h"
#include "../src/distance.h"
#include "../src/grid.h"

using namespace parking;

TEST_CASE(test_bfs_road_distances_and_slot_distances) {
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
    CHECK_EQ(entrance_idx, 0);

    std::vector<char> grid_data(static_cast<size_t>(rows * cols));
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            grid_data[static_cast<size_t>(r * cols + c)] = map_lines[static_cast<size_t>(r)][static_cast<size_t>(c)];
        }
    }

    std::vector<int32_t> road_dist = distance::compute_road_distances(rows, cols, grid_data, entrance_idx);
    
    // Check entrance
    CHECK_EQ(road_dist[0], 0);
    // (1, 2) is road '.', dist 1
    CHECK_EQ(road_dist[1], 1);
    // (1, 6) is road '.', dist 5
    CHECK_EQ(road_dist[5], 5);
    // (2, 6) is road '.', dist 6
    CHECK_EQ(road_dist[1 * cols + 5], 6);

    // Slot (2, 1) touches E(1, 1) at road_dist 0 -> slot dist 1
    int32_t slot_2_1 = 1 * cols + 0;
    CHECK_EQ(distance::compute_slot_distance(slot_2_1, rows, cols, grid_data, road_dist), 1);

    // Slot (2, 3) touches (1, 3) at road_dist 2 -> slot dist 3
    int32_t slot_2_3 = 1 * cols + 2;
    CHECK_EQ(distance::compute_slot_distance(slot_2_3, rows, cols, grid_data, road_dist), 3);

    // Slot (1, 7) touches (1, 6) at road_dist 5 -> slot dist 6... wait!
    // In (1, 7): touches (1, 6) which is '.', dist is 5. Wait, 5 + 1 = 6!
    // Let's verify slot (1, 7):
    // E is (1, 1) = 0.
    // (1, 2) = 1
    // (1, 3) = 2
    // (1, 4) = 3
    // (1, 5) = 4
    // (1, 6) = 5
    // Slot (1, 7) is adjacent to (1, 6) -> dist = 5 + 1 = 6.
    int32_t slot_1_7 = 0 * cols + 6;
    CHECK_EQ(distance::compute_slot_distance(slot_1_7, rows, cols, grid_data, road_dist), 6);
}

TEST_CASE(test_unreachable_slot) {
    int32_t rows = 3;
    int32_t cols = 3;
    std::vector<std::string> map_lines = {
        "E.#",
        ".##",
        "##P" // P is surrounded by #
    };
    int32_t entrance_idx = 0;
    std::vector<char> grid_data(9);
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            grid_data[static_cast<size_t>(r * cols + c)] = map_lines[static_cast<size_t>(r)][static_cast<size_t>(c)];
        }
    }

    auto road_dist = distance::compute_road_distances(rows, cols, grid_data, entrance_idx);
    int32_t slot_p = 2 * cols + 2;
    CHECK_EQ(distance::compute_slot_distance(slot_p, rows, cols, grid_data, road_dist), -1);
}

TEST_CASE(test_slot_reachable_only_through_other_slot) {
    // Spec: cars cannot cross slots
    int32_t rows = 3;
    int32_t cols = 3;
    std::vector<std::string> map_lines = {
        "E.#",
        ".#P",
        "##P" // bottom P only touches right P, not road
    };
    int32_t entrance_idx = 0;
    std::vector<char> grid_data(9);
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            grid_data[static_cast<size_t>(r * cols + c)] = map_lines[static_cast<size_t>(r)][static_cast<size_t>(c)];
        }
    }

    auto road_dist = distance::compute_road_distances(rows, cols, grid_data, entrance_idx);
    int32_t slot_bottom_p = 2 * cols + 2;
    CHECK_EQ(distance::compute_slot_distance(slot_bottom_p, rows, cols, grid_data, road_dist), -1);
}

int main() {
    return test_framework::run_all_tests();
}

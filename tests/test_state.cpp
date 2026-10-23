#include "test_framework.h"
#include "../src/state.h"
#include "../src/slots.h"
#include "../src/distance.h"
#include "../src/grid.h"

using namespace parking;

static LotState make_sample_lot() {
    int32_t rows = 5;
    int32_t cols = 7;
    std::vector<std::string> map_lines = {
        "E.....P",
        "P#P#P.P",
        "P.....#",
        "PP#P#.P",
        "P####.P"
    };

    int32_t entrance_idx = 0;
    std::vector<char> grid_data(static_cast<size_t>(rows * cols));
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            grid_data[static_cast<size_t>(r * cols + c)] = map_lines[static_cast<size_t>(r)][static_cast<size_t>(c)];
        }
    }

    auto road_dist = distance::compute_road_distances(rows, cols, grid_data, entrance_idx);
    LotState lot;
    lot.created = true;
    lot.rows = rows;
    lot.cols = cols;
    lot.total_cells = static_cast<size_t>(rows * cols);
    lot.grid = grid_data;
    slots::build_usable_slots(rows, cols, grid_data, road_dist, lot);
    return lot;
}

TEST_CASE(test_park_and_leave_state) {
    LotState lot = make_sample_lot();

    CHECK(!state::is_registration_parked(lot, "KA-01-HH-1234"));
    CHECK_EQ(lot.free_ranks.size(), static_cast<size_t>(10));

    // Park first car -> should get rank 0 (2, 1)
    Position p1 = state::park_car(lot, "KA-01-HH-1234", "White");
    CHECK_EQ(p1.row, 2);
    CHECK_EQ(p1.col, 1);
    CHECK(state::is_registration_parked(lot, "KA-01-HH-1234"));
    CHECK_EQ(lot.free_ranks.size(), static_cast<size_t>(9));
    CHECK_EQ(lot.all_occupied.size(), static_cast<size_t>(1));

    // Park second car -> should get rank 1 (2, 3)
    Position p2 = state::park_car(lot, "KA-01-HH-9999", "white"); // lowercase colour
    CHECK_EQ(p2.row, 2);
    CHECK_EQ(p2.col, 3);
    CHECK_EQ(lot.free_ranks.size(), static_cast<size_t>(8));

    // Check colour index: "white" should have both cells
    CHECK_EQ(lot.colour_to_cells["white"].size(), static_cast<size_t>(2));

    // Free first car (2, 1) -> cell index 1*7 + 0 = 7
    int32_t cell_2_1 = 1 * lot.cols + 0;
    CHECK(state::is_slot_occupied(lot, cell_2_1));
    state::free_slot(lot, cell_2_1);
    CHECK(!state::is_slot_occupied(lot, cell_2_1));
    CHECK(!state::is_registration_parked(lot, "KA-01-HH-1234"));
    CHECK_EQ(lot.colour_to_cells["white"].size(), static_cast<size_t>(1));

    // Park third car -> should reuse freed rank 0 (2, 1) because it has the smallest rank!
    Position p3 = state::park_car(lot, "KA-01-BB-0001", "Black");
    CHECK_EQ(p3.row, 2);
    CHECK_EQ(p3.col, 1);

    // Re-parking the same registration that previously left
    Position p4 = state::park_car(lot, "KA-01-HH-1234", "Red");
    CHECK_EQ(p4.row, 2);
    CHECK_EQ(p4.col, 5); // rank 2 is (2, 5)
}

TEST_CASE(test_colour_empty_set_removed) {
    LotState lot = make_sample_lot();
    state::park_car(lot, "CAR-1", "Magenta");
    CHECK(lot.colour_to_cells.find("magenta") != lot.colour_to_cells.end());

    int32_t cell = lot.reg_to_cell["CAR-1"];
    state::free_slot(lot, cell);
    // After only car of Magenta leaves, set is cleaned up
    CHECK(lot.colour_to_cells.find("magenta") == lot.colour_to_cells.end());
}

int main() {
    return test_framework::run_all_tests();
}

#include "test_framework.h"
#include "../src/parser.h"
#include "../src/commands.h"
#include "../src/formatter.h"
#include "../src/grid.h"
#include "../src/distance.h"
#include "../src/slots.h"
#include "../src/state.h"

using namespace parking;

static std::string run_script(std::string_view input) {
    auto cmds = parser::parse_input(input);
    LotState lot;
    std::string out;
    for (const auto& cmd : cmds) {
        CommandResult res = commands::execute(lot, cmd);
        formatter::format_result(res, out);
    }
    return out;
}

// A. Program / file level
TEST_CASE(test_edge_case_3_empty_file) {
    std::string out = run_script("");
    CHECK_EQ(out, "");
}

TEST_CASE(test_edge_case_4_blank_lines_only) {
    std::string out = run_script("   \n\n\t  \r\n\n");
    CHECK_EQ(out, "");
}

TEST_CASE(test_edge_case_5_crlf_tabs_spaces) {
    std::string script = "create_parking_lot \t 1 \t 2 \r\nE. \r\npark \t C1 \t Red\r\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 0 usable slots\n") != std::string::npos);
    CHECK(out.find("Sorry, parking lot is full\n") != std::string::npos);
}

TEST_CASE(test_edge_case_6_utf8_bom) {
    std::string script = "\xEF\xBB\xBF" "create_parking_lot 1 2\nEP\npark C1 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_7_no_trailing_newline) {
    std::string script = "create_parking_lot 1 2\nEP\npark C1 Blue";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_8_non_ascii_bytes) {
    std::string script = "create_parking_lot 1 2\nEP\npark KA-\xC3\xA9-1234 R\xC3\xB8" "d\nstatus\n";
    std::string out = run_script(script);
    CHECK(out.find("KA-\xC3\xA9-1234") != std::string::npos);
    CHECK(out.find("R\xC3\xB8" "d") != std::string::npos);
}

// B. Command parsing
TEST_CASE(test_edge_case_10_unknown_command) {
    std::string out = run_script("unknown_cmd 1 2 3\n");
    CHECK_EQ(out, "Error: unknown command 'unknown_cmd'\n");
}

TEST_CASE(test_edge_case_11_wrong_case_command) {
    std::string out = run_script("PARK C1 Blue\nStatus\n");
    CHECK_EQ(out, "Error: unknown command 'PARK'\nError: unknown command 'Status'\n");
}

TEST_CASE(test_edge_case_12_missing_arguments) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park\n"
        "park C1\n"
        "leave\n"
        "leave 1\n"
        "registration_numbers_for_cars_with_colour\n"
        "slot_numbers_for_cars_with_colour\n"
        "slot_number_for_registration_number\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: park failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: leave failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: registration_numbers_for_cars_with_colour failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: slot_numbers_for_cars_with_colour failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: slot_number_for_registration_number failed: invalid arguments\n") != std::string::npos);
}

TEST_CASE(test_edge_case_13_extra_arguments) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Blue Extra\n"
        "status extra_arg\n"
        "leave 1 2 3\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: park failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: status failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: leave failed: invalid arguments\n") != std::string::npos);
}

TEST_CASE(test_edge_case_14_command_before_create) {
    std::string script = 
        "park C1 Blue\n"
        "leave 1 1\n"
        "status\n"
        "registration_numbers_for_cars_with_colour Blue\n"
        "slot_numbers_for_cars_with_colour Blue\n"
        "slot_number_for_registration_number C1\n";
    std::string out = run_script(script);
    std::string expected = 
        "Error: park failed: parking lot not created\n"
        "Error: leave failed: parking lot not created\n"
        "Error: status failed: parking lot not created\n"
        "Error: registration_numbers_for_cars_with_colour failed: parking lot not created\n"
        "Error: slot_numbers_for_cars_with_colour failed: parking lot not created\n"
        "Error: slot_number_for_registration_number failed: parking lot not created\n";
    CHECK_EQ(out, expected);
}

TEST_CASE(test_edge_case_15_very_long_lines) {
    std::string long_reg(300, 'X'); // over 256 bytes
    std::string script = "create_parking_lot 1 2\nEP\npark " + long_reg + " Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: park failed: invalid arguments\n") != std::string::npos);
}

// C. create_parking_lot
TEST_CASE(test_edge_case_16_missing_or_extra_dimension_args) {
    std::string out1 = run_script("create_parking_lot 5\n");
    CHECK_EQ(out1, "Error: create_parking_lot failed: invalid arguments\n");

    std::string out2 = run_script("create_parking_lot 5 5 5\n");
    CHECK_EQ(out2, "Error: create_parking_lot failed: invalid arguments\n");
}

TEST_CASE(test_edge_case_17_invalid_dimensions) {
    std::vector<std::string> invalid_cmds = {
        "create_parking_lot abc 5\n",
        "create_parking_lot 5 -2\n",
        "create_parking_lot 0 5\n",
        "create_parking_lot 2.5 5\n",
        "create_parking_lot +3 5\n"
    };
    for (const auto& cmd : invalid_cmds) {
        std::string out = run_script(cmd);
        CHECK_EQ(out, "Error: create_parking_lot failed: invalid arguments\n");
    }
}

TEST_CASE(test_edge_case_18_grid_too_large) {
    std::string out = run_script("create_parking_lot 10000 2001\n"); // 20,010,000 > 20,000,000
    CHECK_EQ(out, "Error: create_parking_lot failed: grid too large\n");
}

TEST_CASE(test_edge_case_19_fewer_map_lines_eof) {
    std::string script = "create_parking_lot 3 3\nE..\n..#\n";
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: create_parking_lot failed: invalid map\n");
}

TEST_CASE(test_edge_case_20_map_line_length_mismatch) {
    std::string script = "create_parking_lot 2 3\nE..\n.. \n";
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: create_parking_lot failed: invalid map\n");
}

TEST_CASE(test_edge_case_21_illegal_chars_in_map) {
    std::string script = "create_parking_lot 2 2\nE.\n.e\n"; // lowercase 'e'
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: create_parking_lot failed: invalid map\n");

    std::string script2 = "create_parking_lot 2 2\nE.\n.1\n"; // digit
    std::string out2 = run_script(script2);
    CHECK_EQ(out2, "Error: create_parking_lot failed: invalid map\n");
}

TEST_CASE(test_edge_case_22_zero_or_multiple_E) {
    std::string no_e = "create_parking_lot 2 2\n..\n..\n";
    CHECK_EQ(run_script(no_e), "Error: create_parking_lot failed: invalid map\n");

    std::string two_e = "create_parking_lot 2 2\nE.\n.E\n";
    CHECK_EQ(run_script(two_e), "Error: create_parking_lot failed: invalid map\n");
}

TEST_CASE(test_edge_case_23_invalid_map_does_not_disturb_existing_lot) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Blue\n"
        "create_parking_lot 2 2\nE.\n.E\n" // second create
        "status\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: create_parking_lot failed: lot already exists\n") != std::string::npos);
    CHECK(out.find("C1") != std::string::npos);
}

TEST_CASE(test_edge_case_24_second_create_consumes_map_block) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "create_parking_lot 2 2\nE.\n.P\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: create_parking_lot failed: lot already exists\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_25_invalid_dimensions_swallows_orphaned_map) {
    // Per rule 3.h & A14: after failed create, orphaned map lines are swallowed silently
    std::string script = 
        "create_parking_lot abc def\n"
        "E..\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: create_parking_lot failed: invalid arguments\n") != std::string::npos);
    CHECK(out.find("Error: park failed: parking lot not created\n") != std::string::npos);
    CHECK(out.find("Error: unknown command 'E..'\n") == std::string::npos);
}

TEST_CASE(test_edge_case_26_valid_map_zero_usable_slots) {
    std::string script = 
        "create_parking_lot 2 2\nE#\n##\n"
        "park C1 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 0 usable slots\n") != std::string::npos);
    CHECK(out.find("Sorry, parking lot is full\n") != std::string::npos);
}

TEST_CASE(test_edge_case_27_tiny_grids) {
    std::string script1 = "create_parking_lot 1 1\nE\npark C1 Red\n";
    std::string out1 = run_script(script1);
    CHECK(out1.find("Created a parking lot with 0 usable slots\n") != std::string::npos);

    std::string script2 = "create_parking_lot 1 3\nE.P\npark C1 Red\n";
    std::string out2 = run_script(script2);
    CHECK(out2.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out2.find("Allocated slot: (1, 3)\n") != std::string::npos);

    std::string script3 = "create_parking_lot 3 1\nE\n.\nP\npark C1 Red\n";
    std::string out3 = run_script(script3);
    CHECK(out3.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out3.find("Allocated slot: (3, 1)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_28_entrance_positions) {
    std::string script = 
        "create_parking_lot 3 3\n"
        "P.P\n"
        ".E.\n"
        "P.P\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 4 usable slots\n") != std::string::npos);
}

TEST_CASE(test_edge_case_29_p_adjacent_to_e) {
    std::string script = "create_parking_lot 1 2\nEP\npark C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_30_p_adjacent_to_multiple_roads) {
    std::string script = 
        "create_parking_lot 3 3\n"
        "E..\n"
        "#P.\n"
        "...\n";
    auto cmds = parser::parse_input(script);
    LotState lot;
    commands::execute(lot, cmds[0]);
    CHECK_EQ(lot.usable_slots[0].distance, 2);
}

TEST_CASE(test_edge_case_31_p_reachable_only_through_other_p) {
    std::string script = "create_parking_lot 1 3\nEPP\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
}

TEST_CASE(test_edge_case_32_p_enclosed_by_walls) {
    std::string script = 
        "create_parking_lot 3 3\n"
        "E.#\n"
        ".##\n"
        "##P\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 0 usable slots\n") != std::string::npos);
}

TEST_CASE(test_edge_case_33_disconnected_roads_ignored) {
    std::string script = 
        "create_parking_lot 3 3\n"
        "E.#\n"
        "###\n"
        "..P\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 0 usable slots\n") != std::string::npos);
}

TEST_CASE(test_edge_case_34_equal_distances_tie_breaking) {
    std::string script = 
        "create_parking_lot 1 3\n"
        "PEP\n"
        "park C1 Red\n"
        "park C2 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 1)\nAllocated slot: (1, 3)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_35_long_snake_maze) {
    int rows = 10;
    int cols = 10;
    std::vector<std::string> maze(rows, std::string(cols, '#'));
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c) maze[r][c] = '.';
        } else {
            if ((r / 2) % 2 == 0) {
                maze[r][cols - 1] = '.';
            } else {
                maze[r][0] = '.';
            }
        }
    }
    maze[0][0] = 'E';
    maze[rows - 1][cols - 1] = 'P';

    std::string script = "create_parking_lot 10 10\n";
    for (int r = 0; r < rows; ++r) {
        script += maze[r] + "\n";
    }
    script += "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (10, 10)\n") != std::string::npos);
}

// D. park
TEST_CASE(test_edge_case_36_lot_full) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Red\n"
        "park C2 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Sorry, parking lot is full\n") != std::string::npos);
}

TEST_CASE(test_edge_case_37_duplicate_registration) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n.P\n"
        "park CAR-1 Red\n"
        "park CAR-1 Blue\n" // duplicate
        "park CAR-2 Green\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: park failed: registration already parked\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (2, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_38_colour_case_insensitivity_display_as_typed) {
    std::string script = 
        "create_parking_lot 1 3\nE.P\n"
        "park C1 WhItE\n"
        "status\n"
        "registration_numbers_for_cars_with_colour white\n"
        "registration_numbers_for_cars_with_colour WHITE\n";
    std::string out = run_script(script);
    CHECK(out.find("WhItE") != std::string::npos);
    CHECK(out.find("C1\nC1\n") != std::string::npos);
}

TEST_CASE(test_edge_case_39_freed_slot_reused_best_not_most_recent) {
    std::string script = 
        "create_parking_lot 1 4\nEPPP\n"
        "park C1 Red\n"  // slot (1, 2) rank 0
        "park C2 Blue\n" // slot (1, 3) rank 1
        "park C3 Grey\n" // slot (1, 4) rank 2
        "leave 1 3\n"
        "leave 1 2\n"
        "park C4 Gold\n";
    std::string out = run_script(script);
    size_t last_alloc = out.rfind("Allocated slot:");
    CHECK(out.substr(last_alloc).find("(1, 2)") != std::string::npos);
}

TEST_CASE(test_edge_case_40_repark_same_reg_after_leave) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Red\n"
        "leave 1 2\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 2)\nSlot (1, 2) is free\nAllocated slot: (1, 2)\n") != std::string::npos);
}

// E. leave
TEST_CASE(test_edge_case_41_leave_invalid_arguments) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n..\n"
        "leave abc 1\n"
        "leave 1 -1\n"
        "leave 0 1\n"
        "leave 1 2.5\n"
        "leave +1 2\n"
        "leave (1, 2)\n"
        "leave 1,2\n";
    std::string out = run_script(script);
    size_t count = 0;
    size_t pos = 0;
    while ((pos = out.find("Error: leave failed: invalid arguments\n", pos)) != std::string::npos) {
        count++;
        pos += 30;
    }
    CHECK_EQ(count, static_cast<size_t>(7));
}

TEST_CASE(test_edge_case_42_leave_slot_out_of_range) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n..\n"
        "leave 5 2\n"
        "leave 999999999999999999999999999999 1\n";
    std::string out = run_script(script);
    size_t pos1 = out.find("Error: leave failed: slot out of range\n");
    CHECK(pos1 != std::string::npos);
    size_t pos2 = out.find("Error: leave failed: slot out of range\n", pos1 + 30);
    CHECK(pos2 != std::string::npos);
}

TEST_CASE(test_edge_case_43_leave_not_usable_slot) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n.#\n"
        "leave 1 1\n"  // entrance
        "leave 2 1\n"  // road
        "leave 2 2\n"; // wall
    std::string out = run_script(script);
    CHECK(out.find("Error: leave failed: not a usable slot\n") != std::string::npos);
}

TEST_CASE(test_edge_case_44_and_46_leave_already_free_no_duplicate_rank) {
    std::string script = 
        "create_parking_lot 2 2\n"
        "EP\n"
        ".P\n"
        "leave 1 2\n" // leave free slot
        "park C1 Red\n"
        "park C2 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: leave failed: slot already free\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (2, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_45_valid_leave_cleans_indexes) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Red\n"
        "leave 1 2\n"
        "status\n"
        "registration_numbers_for_cars_with_colour Red\n"
        "slot_number_for_registration_number C1\n";
    std::string out = run_script(script);
    CHECK(out.find("Slot (1, 2) is free\n") != std::string::npos);
    CHECK(out.find("Parking lot is empty\n") != std::string::npos);
    CHECK(out.find("Not found\nNot found\n") != std::string::npos);
}

// F. status
TEST_CASE(test_edge_case_47_status_empty_lot_no_header) {
    std::string script = "create_parking_lot 1 2\nEP\nstatus\n";
    std::string out = run_script(script);
    CHECK(out.find("Parking lot is empty\n") != std::string::npos);
    CHECK(out.find("Slot     Registration") == std::string::npos);
}

TEST_CASE(test_edge_case_48_and_49_status_ordering_row_then_column) {
    std::string script = 
        "create_parking_lot 2 3\n"
        "E.P\n"
        "P##\n"
        "park C1 Red\n"  // parked at (2, 1)
        "park C2 Blue\n" // parked at (1, 3)
        "status\n";
    std::string out = run_script(script);
    size_t status_pos = out.find("Slot     Registration No");
    CHECK(status_pos != std::string::npos);
    std::string status_table = out.substr(status_pos);
    size_t pos_c2 = status_table.find("(1, 3)");
    size_t pos_c1 = status_table.find("(2, 1)");
    CHECK(pos_c2 != std::string::npos);
    CHECK(pos_c1 != std::string::npos);
    CHECK(pos_c2 < pos_c1);
}

// G. Lookups
TEST_CASE(test_edge_case_51_52_53_54_lookups) {
    std::string script = 
        "create_parking_lot 2 3\n"
        "E.P\n"
        "P.P\n"
        "registration_numbers_for_cars_with_colour Pink\n"
        "park C1 RED\n"
        "park C2 red\n"
        "registration_numbers_for_cars_with_colour Red\n"
        "slot_numbers_for_cars_with_colour Red\n"
        "slot_number_for_registration_number C1\n"
        "slot_number_for_registration_number c1\n";
    std::string out = run_script(script);
    CHECK(out.find("Not found\n") != std::string::npos);
    CHECK(out.find("C1, C2\n") != std::string::npos || out.find("C2, C1\n") != std::string::npos);
    size_t last_nf = out.rfind("Not found\n");
    CHECK(last_nf != std::string::npos);
}

TEST_CASE(test_edge_case_60_no_partial_mutation_on_failure) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Red\n"
        "park C1 Blue\n" // duplicate registration
        "status\n";
    std::string out = run_script(script);
    CHECK(out.find("Red") != std::string::npos);
    CHECK(out.find("Blue") == std::string::npos);
}

int main() {
    return test_framework::run_all_tests();
}

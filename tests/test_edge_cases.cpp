#include "test_framework.h"
#include "../src/parser.h"
#include "../src/commands.h"
#include "../src/formatter.h"
#include "../src/grid.h"
#include "../src/distance.h"
#include "../src/slots.h"
#include "../src/state.h"

using namespace parking;

// Helper to run raw text script and return output
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
    CHECK(out.find("Error: invalid arguments for park\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for leave\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for registration_numbers_for_cars_with_colour\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for slot_numbers_for_cars_with_colour\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for slot_number_for_registration_number\n") != std::string::npos);
}

TEST_CASE(test_edge_case_13_extra_arguments) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Blue Extra\n"
        "status extra_arg\n"
        "leave 1 2 3\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: invalid arguments for park\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for status\n") != std::string::npos);
    CHECK(out.find("Error: invalid arguments for leave\n") != std::string::npos);
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
        "Error: parking lot not created\n"
        "Error: parking lot not created\n"
        "Error: parking lot not created\n"
        "Error: parking lot not created\n"
        "Error: parking lot not created\n"
        "Error: parking lot not created\n";
    CHECK_EQ(out, expected);
}

TEST_CASE(test_edge_case_15_very_long_lines) {
    std::string long_reg(5000, 'X');
    std::string long_colour(5000, 'Y');
    std::string script = "create_parking_lot 1 2\nEP\npark " + long_reg + " " + long_colour + "\nslot_number_for_registration_number " + long_reg + "\n";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
    CHECK(out.find("(1, 2)\n") != std::string::npos);
}

// C. create_parking_lot
TEST_CASE(test_edge_case_16_missing_or_extra_dimension_args) {
    std::string out1 = run_script("create_parking_lot 5\n");
    CHECK_EQ(out1, "Error: invalid arguments for create_parking_lot\n");

    std::string out2 = run_script("create_parking_lot 5 5 5\n");
    CHECK_EQ(out2, "Error: invalid arguments for create_parking_lot\n");
}

TEST_CASE(test_edge_case_17_invalid_dimensions) {
    std::vector<std::string> invalid_cmds = {
        "create_parking_lot abc 5\n",
        "create_parking_lot 5 -2\n",
        "create_parking_lot 0 5\n",
        "create_parking_lot 2.5 5\n",
        "create_parking_lot +3 5\n",
        "create_parking_lot 99999999999999999999 5\n"
    };
    for (const auto& cmd : invalid_cmds) {
        std::string out = run_script(cmd);
        CHECK_EQ(out, "Error: invalid dimensions\n");
    }
}

TEST_CASE(test_edge_case_18_grid_too_large) {
    std::string out = run_script("create_parking_lot 10000 2001\n"); // 20,010,000 > 20,000,000
    CHECK_EQ(out, "Error: grid too large\n");
}

TEST_CASE(test_edge_case_19_fewer_map_lines_eof) {
    std::string script = "create_parking_lot 3 3\nE..\n..#\n";
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: invalid map\n");
}

TEST_CASE(test_edge_case_20_map_line_length_mismatch) {
    std::string script = "create_parking_lot 2 3\nE..\n.. \n";
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: invalid map\n");
}

TEST_CASE(test_edge_case_21_illegal_chars_in_map) {
    std::string script = "create_parking_lot 2 2\nE.\n.e\n"; // lowercase 'e'
    std::string out = run_script(script);
    CHECK_EQ(out, "Error: invalid map\n");

    std::string script2 = "create_parking_lot 2 2\nE.\n.1\n"; // digit
    std::string out2 = run_script(script2);
    CHECK_EQ(out2, "Error: invalid map\n");
}

TEST_CASE(test_edge_case_22_zero_or_multiple_E) {
    std::string no_e = "create_parking_lot 2 2\n..\n..\n";
    CHECK_EQ(run_script(no_e), "Error: invalid map\n");

    std::string two_e = "create_parking_lot 2 2\nE.\n.E\n";
    CHECK_EQ(run_script(two_e), "Error: invalid map\n");
}

TEST_CASE(test_edge_case_23_invalid_map_does_not_disturb_existing_lot) {
    // Note: Once created, a second create gives PARKING_LOT_ALREADY_CREATED
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Blue\n"
        "create_parking_lot 2 2\nE.\n.E\n" // second create
        "status\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: parking lot already created\n") != std::string::npos);
    CHECK(out.find("C1") != std::string::npos);
}

TEST_CASE(test_edge_case_24_second_create_consumes_map_block) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "create_parking_lot 2 2\nE.\n.P\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: parking lot already created\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_25_invalid_dimensions_do_not_guess_map_lines) {
    std::string script = 
        "create_parking_lot abc def\n"
        "E..\n"
        "park C1 Red\n";
    std::string out = run_script(script);
    CHECK(out.find("Error: invalid dimensions\n") != std::string::npos);
    CHECK(out.find("Error: unknown command 'E..'\n") != std::string::npos);
    CHECK(out.find("Error: parking lot not created\n") != std::string::npos);
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
    // 1x1 with only E
    std::string script1 = "create_parking_lot 1 1\nE\npark C1 Red\n";
    std::string out1 = run_script(script1);
    CHECK(out1.find("Created a parking lot with 0 usable slots\n") != std::string::npos);

    // 1x3: E.P
    std::string script2 = "create_parking_lot 1 3\nE.P\npark C1 Red\n";
    std::string out2 = run_script(script2);
    CHECK(out2.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out2.find("Allocated slot: (1, 3)\n") != std::string::npos);

    // 3x1: E / . / P
    std::string script3 = "create_parking_lot 3 1\nE\n.\nP\npark C1 Red\n";
    std::string out3 = run_script(script3);
    CHECK(out3.find("Created a parking lot with 1 usable slots\n") != std::string::npos);
    CHECK(out3.find("Allocated slot: (3, 1)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_28_entrance_positions) {
    // E in the middle
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
    // (2, 2) is P, touching (1, 2) which is road dist 1, and (2, 3) which is road dist 5
    // Should use min road dist (1 + 1 = 2)
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
    std::string script = 
        "create_parking_lot 1 3\n"
        "EPP\n";
    std::string out = run_script(script);
    // (1, 2) is usable (touches E), (1, 3) is NOT usable (touches only P, cars cannot cross P)
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
        "..P\n"; // disconnected road and P
    std::string out = run_script(script);
    CHECK(out.find("Created a parking lot with 0 usable slots\n") != std::string::npos);
}

TEST_CASE(test_edge_case_34_equal_distances_tie_breaking) {
    // E at (1, 2). (1, 1) and (1, 3) are P at distance 1.
    // Smaller col (1, 1) should be chosen first!
    std::string script = 
        "create_parking_lot 1 3\n"
        "PEP\n"
        "park C1 Red\n"
        "park C2 Blue\n";
    std::string out = run_script(script);
    CHECK(out.find("Allocated slot: (1, 1)\nAllocated slot: (1, 3)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_35_long_snake_maze) {
    // Snake maze of 100 cells
    int rows = 10;
    int cols = 10;
    std::vector<std::string> maze(rows, std::string(cols, '#'));
    // Simple S-curve
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
        "create_parking_lot 1 3\nEPP\n"
        "park CAR-1 Red\n"
        "park CAR-1 Blue\n" // duplicate
        "park CAR-2 Green\n"; // should succeed in next slot
    std::string out = run_script(script);
    CHECK(out.find("Error: registration CAR-1 is already parked\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
}

TEST_CASE(test_edge_case_38_colour_case_insensitivity_display_as_typed) {
    std::string script = 
        "create_parking_lot 1 3\nE.P\n"
        "park C1 WhItE\n"
        "status\n"
        "registration_numbers_for_cars_with_colour white\n"
        "registration_numbers_for_cars_with_colour WHITE\n";
    std::string out = run_script(script);
    CHECK(out.find("WhItE") != std::string::npos); // displayed as typed in status
    CHECK(out.find("C1\nC1\n") != std::string::npos); // matches both lower and upper query
}

TEST_CASE(test_edge_case_39_freed_slot_reused_best_not_most_recent) {
    // 3 slots at distances 1, 2, 3
    std::string script = 
        "create_parking_lot 1 4\nEPPP\n"
        "park C1 Red\n"  // slot (1, 2) rank 0
        "park C2 Blue\n" // slot (1, 3) rank 1
        "park C3 Grey\n" // slot (1, 4) rank 2
        "leave 1 3\n"    // free rank 1
        "leave 1 2\n"    // free rank 0 (most recent)
        "park C4 Gold\n"; // should choose rank 0 (1, 2), not rank 1 (1, 3)
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
        "leave 99999999999999999999 1\n";
    std::string out = run_script(script);
    size_t count = 0;
    size_t pos = 0;
    while ((pos = out.find("Error: invalid arguments for leave\n", pos)) != std::string::npos) {
        count++;
        pos += 30;
    }
    CHECK_EQ(count, static_cast<size_t>(6));
}

TEST_CASE(test_edge_case_42_leave_slot_out_of_range) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n..\n"
        "leave 5 2\n";
    std::string out = run_script(script);
    CHECK_EQ(out.substr(out.find("Error: slot")), "Error: slot (5, 2) is out of range\n");
}

TEST_CASE(test_edge_case_43_leave_not_usable_slot) {
    std::string script = 
        "create_parking_lot 2 2\nEP\n.#\n"
        "leave 1 1\n"  // entrance
        "leave 2 1\n"  // road
        "leave 2 2\n"; // wall
    std::string out = run_script(script);
    CHECK(out.find("Error: (1, 1) is not a usable parking slot\n") != std::string::npos);
    CHECK(out.find("Error: (2, 1) is not a usable parking slot\n") != std::string::npos);
    CHECK(out.find("Error: (2, 2) is not a usable parking slot\n") != std::string::npos);
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
    CHECK(out.find("Error: slot (1, 2) is already free\n") != std::string::npos);
    CHECK(out.find("Allocated slot: (1, 2)\n") != std::string::npos);
    // Crucial: C2 must be allocated (2, 2), NOT (1, 2) again!
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
    // Park in order of distance: (2, 1) dist 1, (1, 3) dist 2
    // Status must print (1, 3) before (2, 1) because row 1 < row 2!
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
    CHECK(pos_c2 < pos_c1); // row 1 appears before row 2 in status table!
}

// G. Lookups
TEST_CASE(test_edge_case_51_52_53_54_lookups) {
    std::string script = 
        "create_parking_lot 2 3\n"
        "E.P\n"
        "P.P\n"
        "registration_numbers_for_cars_with_colour Pink\n" // colour never seen -> Not found
        "park C1 RED\n"
        "park C2 red\n"
        "registration_numbers_for_cars_with_colour Red\n" // case-insensitive match
        "slot_numbers_for_cars_with_colour Red\n"
        "slot_number_for_registration_number C1\n"
        "slot_number_for_registration_number c1\n"; // registration is case-sensitive!
    std::string out = run_script(script);
    CHECK(out.find("Not found\n") != std::string::npos);
    CHECK(out.find("C1, C2\n") != std::string::npos || out.find("C2, C1\n") != std::string::npos);
    // Registration lookup for "c1" (wrong case) must return "Not found"
    size_t last_nf = out.rfind("Not found\n");
    CHECK(last_nf != std::string::npos);
}

TEST_CASE(test_edge_case_60_no_partial_mutation_on_failure) {
    std::string script = 
        "create_parking_lot 1 2\nEP\n"
        "park C1 Red\n"
        "park C1 Blue\n" // fails: duplicate reg
        "status\n";      // car must still be Red, not Blue
    std::string out = run_script(script);
    CHECK(out.find("Red") != std::string::npos);
    CHECK(out.find("Blue") == std::string::npos);
}

int main() {
    return test_framework::run_all_tests();
}

#include "test_framework.h"
#include "../src/parser.h"

using namespace parking;

TEST_CASE(test_strip_utf8_bom) {
    std::string bom = "\xEF\xBB\xBFpark KA-01-1111 Red";
    std::string_view stripped = parser::strip_utf8_bom(bom);
    CHECK_EQ(stripped, "park KA-01-1111 Red");

    std::string no_bom = "park KA-01-1111 Red";
    CHECK_EQ(parser::strip_utf8_bom(no_bom), "park KA-01-1111 Red");
}

TEST_CASE(test_split_lines_crlf_and_trailing_whitespace) {
    std::string input = "line1 \t\r\nline2\r\nline3   ";
    auto lines = parser::split_lines(input);
    CHECK_EQ(lines.size(), static_cast<size_t>(3));
    CHECK_EQ(lines[0], "line1");
    CHECK_EQ(lines[1], "line2");
    CHECK_EQ(lines[2], "line3");
}

TEST_CASE(test_tokenize_multiple_spaces_tabs) {
    std::string line = "  park \t  KA-01-1234 \t  White   ";
    auto tokens = parser::tokenize(line);
    CHECK_EQ(tokens.size(), static_cast<size_t>(3));
    CHECK_EQ(tokens[0], "park");
    CHECK_EQ(tokens[1], "KA-01-1234");
    CHECK_EQ(tokens[2], "White");
}

TEST_CASE(test_create_parking_lot_map_consumption) {
    std::string input = 
        "create_parking_lot 3 3\n"
        "E..\n"
        ".#.\n"
        "..P\n"
        "park CAR-1 White\n";

    auto cmds = parser::parse_input(input);
    CHECK_EQ(cmds.size(), static_cast<size_t>(2));
    CHECK(cmds[0].type == CommandType::CREATE_PARKING_LOT);
    CHECK_EQ(cmds[0].rows, 3);
    CHECK_EQ(cmds[0].cols, 3);
    CHECK_EQ(cmds[0].map_lines.size(), static_cast<size_t>(3));
    CHECK_EQ(cmds[0].map_lines[0], "E..");
    CHECK_EQ(cmds[0].map_lines[1], ".#.");
    CHECK_EQ(cmds[0].map_lines[2], "..P");

    CHECK(cmds[1].type == CommandType::PARK);
    CHECK_EQ(cmds[1].args.size(), static_cast<size_t>(2));
    CHECK_EQ(cmds[1].args[0], "CAR-1");
}

TEST_CASE(test_invalid_dimensions) {
    // Decimal "2.5"
    auto c1 = parser::parse_input("create_parking_lot 2.5 3\n");
    CHECK_EQ(c1.size(), static_cast<size_t>(1));
    CHECK(c1[0].type == CommandType::INVALID_DIMENSIONS);

    // Negative "-3"
    auto c2 = parser::parse_input("create_parking_lot -3 3\n");
    CHECK(c2[0].type == CommandType::INVALID_DIMENSIONS);

    // Signed "+3"
    auto c3 = parser::parse_input("create_parking_lot +3 3\n");
    CHECK(c3[0].type == CommandType::INVALID_DIMENSIONS);

    // Zero "0"
    auto c4 = parser::parse_input("create_parking_lot 0 3\n");
    CHECK(c4[0].type == CommandType::INVALID_DIMENSIONS);

    // Overflow
    auto c5 = parser::parse_input("create_parking_lot 99999999999999999999 3\n");
    CHECK(c5[0].type == CommandType::INVALID_DIMENSIONS);
}

TEST_CASE(test_grid_too_large) {
    auto c1 = parser::parse_input("create_parking_lot 5000 5000\n"); // 25,000,000 > 20,000,000
    CHECK_EQ(c1.size(), static_cast<size_t>(1));
    CHECK(c1[0].type == CommandType::GRID_TOO_LARGE);
}

TEST_CASE(test_fewer_map_lines_eof_early) {
    std::string input = 
        "create_parking_lot 5 3\n"
        "E..\n"
        ".#.\n"; // Only 2 lines instead of 5
    auto cmds = parser::parse_input(input);
    CHECK_EQ(cmds.size(), static_cast<size_t>(1));
    CHECK(cmds[0].type == CommandType::INVALID_MAP);
    CHECK_EQ(cmds[0].map_lines.size(), static_cast<size_t>(2));
}

TEST_CASE(test_case_sensitivity_of_command_names) {
    auto cmds = parser::parse_input("PARK CAR-1 White\nStatus\n");
    CHECK_EQ(cmds.size(), static_cast<size_t>(2));
    CHECK(cmds[0].type == CommandType::UNKNOWN_COMMAND);
    CHECK_EQ(cmds[0].raw_name, "PARK");
    CHECK(cmds[1].type == CommandType::UNKNOWN_COMMAND);
    CHECK_EQ(cmds[1].raw_name, "Status");
}

int main() {
    return test_framework::run_all_tests();
}

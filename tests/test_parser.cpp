#include "test_framework.h"
#include "../src/parser.h"

using namespace parking;

TEST_CASE(test_split_lines_crlf_lone_cr_lone_lf) {
    std::string input = "line1 \t\r\nline2\rline3\nline4   ";
    auto lines = parser::split_lines(input);
    CHECK_EQ(lines.size(), static_cast<size_t>(4));
    CHECK_EQ(parser::trim_trailing(lines[0]), "line1");
    CHECK_EQ(parser::trim_trailing(lines[1]), "line2");
    CHECK_EQ(parser::trim_trailing(lines[2]), "line3");
    CHECK_EQ(parser::trim_trailing(lines[3]), "line4");
}

TEST_CASE(test_tokenize_multiple_spaces_tabs) {
    std::string line = "  park \t  KA-01-1234 \t  White   ";
    auto tokens = parser::tokenize(line);
    CHECK_EQ(tokens.size(), static_cast<size_t>(3));
    CHECK_EQ(tokens[0], "park");
    CHECK_EQ(tokens[1], "KA-01-1234");
    CHECK_EQ(tokens[2], "White");
}

TEST_CASE(test_token_validation) {
    // Valid token
    CHECK(parser::is_valid_string_token("KA-01-1234"));
    CHECK(parser::is_valid_string_token("White"));
    // Non-ASCII allowed (>= 0x80)
    CHECK(parser::is_valid_string_token("R\xC3\xB8" "d"));

    // Control characters rejected (< 0x20 or 0x7F)
    CHECK(!parser::is_valid_string_token(std::string("CAR\x01-1")));
    CHECK(!parser::is_valid_string_token(std::string("CAR\x7F-1")));
    CHECK(!parser::is_valid_string_token(std::string("CAR\x1F-1")));

    // Empty rejected
    CHECK(!parser::is_valid_string_token(""));

    // Over 256 bytes rejected
    std::string long_tok(257, 'A');
    CHECK(!parser::is_valid_string_token(long_tok));
    std::string ok_tok(256, 'A');
    CHECK(parser::is_valid_string_token(ok_tok));
}

TEST_CASE(test_parse_positive_number_saturating) {
    uint64_t val = 0;
    // Valid digits
    CHECK(parser::parse_positive_number_saturating("5", 100, val) == parser::ParseNumResult::VALID);
    CHECK_EQ(val, 5ULL);

    // Leading zeros allowed
    CHECK(parser::parse_positive_number_saturating("01", 100, val) == parser::ParseNumResult::VALID);
    CHECK_EQ(val, 1ULL);

    // Zero
    CHECK(parser::parse_positive_number_saturating("0", 100, val) == parser::ParseNumResult::ZERO);
    CHECK(parser::parse_positive_number_saturating("000", 100, val) == parser::ParseNumResult::ZERO);

    // Invalid characters (signs, decimals, hex, letters, parens)
    CHECK(parser::parse_positive_number_saturating("+5", 100, val) == parser::ParseNumResult::INVALID_CHARS);
    CHECK(parser::parse_positive_number_saturating("-5", 100, val) == parser::ParseNumResult::INVALID_CHARS);
    CHECK(parser::parse_positive_number_saturating("2.5", 100, val) == parser::ParseNumResult::INVALID_CHARS);
    CHECK(parser::parse_positive_number_saturating("0x5", 100, val) == parser::ParseNumResult::INVALID_CHARS);
    CHECK(parser::parse_positive_number_saturating("(1", 100, val) == parser::ParseNumResult::INVALID_CHARS);
    CHECK(parser::parse_positive_number_saturating("1,", 100, val) == parser::ParseNumResult::INVALID_CHARS);

    // Saturation
    CHECK(parser::parse_positive_number_saturating("999999999999999999999999999999", 20000000ULL, val) == parser::ParseNumResult::VALID);
    CHECK_EQ(val, 20000001ULL);
}

TEST_CASE(test_create_parking_lot_early_stop_on_known_command) {
    // 5 rows declared, but 3rd line is 'status' (known command)
    std::string input = 
        "create_parking_lot 5 3\n"
        "E..\n"
        "..P\n"
        "status\n";
    auto cmds = parser::parse_input(input);
    CHECK_EQ(cmds.size(), static_cast<size_t>(2));
    CHECK(cmds[0].type == CommandType::CREATE_PARKING_LOT);
    CHECK(!cmds[0].map_is_valid); // early stop -> invalid map
    CHECK(cmds[0].error_reason == ErrorReason::INVALID_MAP);
    CHECK(cmds[1].type == CommandType::STATUS);
}

TEST_CASE(test_unexpected_map_data) {
    std::string input = 
        "create_parking_lot 1 2\n"
        "EP\n"
        "..\n"
        "##\n"
        "park C1 Red\n";
    auto cmds = parser::parse_input(input);
    CHECK_EQ(cmds.size(), static_cast<size_t>(3));
    CHECK(cmds[0].type == CommandType::CREATE_PARKING_LOT);
    CHECK(cmds[0].map_is_valid);
    CHECK(cmds[1].type == CommandType::UNEXPECTED_MAP_DATA);
    CHECK(cmds[2].type == CommandType::PARK);
}

TEST_CASE(test_orphaned_map_silently_swallowed_on_failed_create) {
    std::string input = 
        "create_parking_lot invalid_args\n"
        "E..\n"
        "...\n"
        "..P\n"
        "status\n";
    auto cmds = parser::parse_input(input);
    CHECK_EQ(cmds.size(), static_cast<size_t>(2));
    CHECK(cmds[0].type == CommandType::PRE_FAILED);
    CHECK(cmds[0].error_reason == ErrorReason::INVALID_ARGUMENTS);
    CHECK(cmds[1].type == CommandType::STATUS);
}

int main() {
    return test_framework::run_all_tests();
}

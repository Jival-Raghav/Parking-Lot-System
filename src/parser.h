#pragma once

#include "types.h"
#include <string>
#include <string_view>
#include <vector>

namespace parking::parser {

enum class ParseNumResult {
    VALID,
    ZERO,
    INVALID_CHARS
};

// Check if a string token is valid (<= 256 bytes, no control chars < 0x20 or 0x7F)
bool is_valid_string_token(std::string_view tok);

// Saturating unsigned number parser (digits 0-9 only, caps at cap + 1)
ParseNumResult parse_positive_number_saturating(std::string_view token, uint64_t cap, uint64_t& out_val);

// Checks if a token matches one of the 7 known commands
bool is_known_command(std::string_view tok);

// Checks if a trimmed line consists only of characters E, P, ., #
bool is_map_like_line(std::string_view raw_line);

// Parse entire input text into a sequence of Commands.
std::vector<Command> parse_input(std::string_view input_text);

// Helper to split text into lines, handling \r\n, lone \n, and lone \r
std::vector<std::string_view> split_lines(std::string_view text);

// Helper to tokenize a line on any run of spaces/tabs
std::vector<std::string_view> tokenize(std::string_view line);

// Trims trailing spaces, tabs, and '\r'
std::string_view trim_trailing(std::string_view s);

} // namespace parking::parser

#pragma once

#include "types.h"
#include <string>
#include <string_view>
#include <vector>

namespace parking::parser {

// Parse entire input text into a sequence of Commands.
// Consumes map lines after valid create_parking_lot.
// Blank lines between commands are ignored.
std::vector<Command> parse_input(std::string_view input_text);

// Helper to strip UTF-8 BOM if present
std::string_view strip_utf8_bom(std::string_view str);

// Helper to split text into lines, trimming '\r' and trailing spaces/tabs
std::vector<std::string> split_lines(std::string_view text);

// Helper to tokenize a line by any run of spaces/tabs
std::vector<std::string> tokenize(std::string_view line);

} // namespace parking::parser

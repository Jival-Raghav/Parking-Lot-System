#include "parser.h"
#include <charconv>
#include <limits>
#include <cctype>

namespace parking::parser {

std::string_view strip_utf8_bom(std::string_view str) {
    if (str.size() >= 3 &&
        static_cast<unsigned char>(str[0]) == 0xEF &&
        static_cast<unsigned char>(str[1]) == 0xBB &&
        static_cast<unsigned char>(str[2]) == 0xBF) {
        return str.substr(3);
    }
    return str;
}

std::vector<std::string> split_lines(std::string_view text) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        std::string_view line_view;
        if (end == std::string_view::npos) {
            line_view = text.substr(start);
            start = text.size();
        } else {
            line_view = text.substr(start, end - start);
            start = end + 1;
        }

        // Strip '\r' if present
        if (!line_view.empty() && line_view.back() == '\r') {
            line_view.remove_suffix(1);
        }

        // Strip trailing spaces and tabs
        while (!line_view.empty() && (line_view.back() == ' ' || line_view.back() == '\t')) {
            line_view.remove_suffix(1);
        }

        lines.emplace_back(line_view);
    }
    return lines;
}

std::vector<std::string> tokenize(std::string_view line) {
    std::vector<std::string> tokens;
    size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
        if (i >= line.size()) break;
        size_t token_start = i;
        while (i < line.size() && line[i] != ' ' && line[i] != '\t') {
            ++i;
        }
        tokens.emplace_back(line.substr(token_start, i - token_start));
    }
    return tokens;
}

namespace {

bool parse_dimension(std::string_view str, int64_t& out_val) {
    if (str.empty()) return false;
    for (char c : str) {
        if (c < '0' || c > '9') return false;
    }
    long long val = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
        return false;
    }
    if (val <= 0) {
        return false;
    }
    out_val = val;
    return true;
}

bool is_grid_too_large(int64_t rows, int64_t cols) {
    constexpr int64_t MAX_CELLS = 20000000LL;
    if (rows <= 0 || cols <= 0) return false;
    if (rows > MAX_CELLS / cols) {
        return true;
    }
    return false;
}

} // anonymous namespace

std::vector<Command> parse_input(std::string_view input_text) {
    std::string_view stripped = strip_utf8_bom(input_text);
    std::vector<std::string> lines = split_lines(stripped);
    std::vector<Command> commands;

    size_t line_idx = 0;
    while (line_idx < lines.size()) {
        const std::string& line = lines[line_idx];
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) {
            // Blank lines between commands are ignored silently
            line_idx++;
            continue;
        }

        const std::string& cmd_name = tokens[0];

        if (cmd_name == "create_parking_lot") {
            if (tokens.size() != 3) {
                // Missing or extra dimension arguments
                Command cmd;
                cmd.type = CommandType::INVALID_ARGS;
                cmd.raw_name = cmd_name;
                commands.push_back(cmd);
                line_idx++;
                continue;
            }

            int64_t rows = 0;
            int64_t cols = 0;
            if (!parse_dimension(tokens[1], rows) || !parse_dimension(tokens[2], cols)) {
                // Invalid dimensions (non-numeric, negative, 0, decimal, signed, overflow)
                Command cmd;
                cmd.type = CommandType::INVALID_DIMENSIONS;
                cmd.raw_name = cmd_name;
                commands.push_back(cmd);
                line_idx++;
                continue;
            }

            if (is_grid_too_large(rows, cols)) {
                Command cmd;
                cmd.type = CommandType::GRID_TOO_LARGE;
                cmd.raw_name = cmd_name;
                commands.push_back(cmd);
                line_idx++;
                continue;
            }

            // Valid dimensions within cap: consume exactly `rows` lines as the map
            size_t available = (line_idx + 1 < lines.size()) ? (lines.size() - (line_idx + 1)) : 0;
            if (available < static_cast<size_t>(rows)) {
                // Fewer map lines than declared rows (EOF early): consume what exists
                Command cmd;
                cmd.type = CommandType::INVALID_MAP;
                cmd.raw_name = cmd_name;
                cmd.rows = rows;
                cmd.cols = cols;
                for (size_t i = line_idx + 1; i < lines.size(); ++i) {
                    cmd.map_lines.push_back(lines[i]);
                }
                line_idx = lines.size();
                commands.push_back(cmd);
                break;
            }

            // Consume exactly `rows` lines
            Command cmd;
            cmd.type = CommandType::CREATE_PARKING_LOT;
            cmd.raw_name = cmd_name;
            cmd.rows = rows;
            cmd.cols = cols;
            cmd.map_lines.reserve(static_cast<size_t>(rows));
            for (int64_t r = 0; r < rows; ++r) {
                line_idx++;
                cmd.map_lines.push_back(lines[line_idx]);
            }
            commands.push_back(std::move(cmd));
            line_idx++;
            continue;
        }

        // Other commands
        Command cmd;
        cmd.raw_name = cmd_name;
        for (size_t t = 1; t < tokens.size(); ++t) {
            cmd.args.push_back(tokens[t]);
        }

        if (cmd_name == "park") {
            cmd.type = CommandType::PARK;
        } else if (cmd_name == "leave") {
            cmd.type = CommandType::LEAVE;
        } else if (cmd_name == "status") {
            cmd.type = CommandType::STATUS;
        } else if (cmd_name == "registration_numbers_for_cars_with_colour") {
            cmd.type = CommandType::REG_NUMS_FOR_COLOUR;
        } else if (cmd_name == "slot_numbers_for_cars_with_colour") {
            cmd.type = CommandType::SLOT_NUMS_FOR_COLOUR;
        } else if (cmd_name == "slot_number_for_registration_number") {
            cmd.type = CommandType::SLOT_FOR_REG;
        } else {
            cmd.type = CommandType::UNKNOWN_COMMAND;
        }

        commands.push_back(std::move(cmd));
        line_idx++;
    }

    return commands;
}

} // namespace parking::parser

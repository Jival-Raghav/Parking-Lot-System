#include "parser.h"
#include <algorithm>

namespace parking::parser {

bool is_valid_string_token(std::string_view tok) {
    if (tok.empty() || tok.size() > 256) return false;
    for (char c : tok) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || uc == 0x7F) return false;
    }
    return true;
}

ParseNumResult parse_positive_number_saturating(std::string_view token, uint64_t cap, uint64_t& out_val) {
    if (token.empty() || token.size() > 256) return ParseNumResult::INVALID_CHARS;
    for (char c : token) {
        if (c < '0' || c > '9') return ParseNumResult::INVALID_CHARS;
    }
    uint64_t val = 0;
    bool all_zeros = true;
    for (char c : token) {
        if (c != '0') all_zeros = false;
        if (val < cap + 1) {
            val = val * 10 + static_cast<uint64_t>(c - '0');
            if (val > cap + 1) {
                val = cap + 1;
            }
        }
    }
    if (all_zeros) return ParseNumResult::ZERO;
    out_val = val;
    return ParseNumResult::VALID;
}

bool is_known_command(std::string_view tok) {
    return tok == "create_parking_lot" ||
           tok == "park" ||
           tok == "leave" ||
           tok == "status" ||
           tok == "registration_numbers_for_cars_with_colour" ||
           tok == "slot_numbers_for_cars_with_colour" ||
           tok == "slot_number_for_registration_number";
}

std::string_view trim_trailing(std::string_view s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) {
        s.remove_suffix(1);
    }
    return s;
}

bool is_map_like_line(std::string_view raw_line) {
    std::string_view trimmed = trim_trailing(raw_line);
    if (trimmed.empty()) return false;
    for (char c : trimmed) {
        if (c != 'E' && c != 'P' && c != '.' && c != '#') return false;
    }
    return true;
}

std::vector<std::string_view> split_lines(std::string_view text) {
    std::vector<std::string_view> lines;
    size_t start = 0;
    size_t n = text.size();
    while (start < n) {
        size_t end = start;
        while (end < n && text[end] != '\n' && text[end] != '\r') {
            ++end;
        }
        lines.push_back(text.substr(start, end - start));
        if (end < n) {
            if (text[end] == '\r' && end + 1 < n && text[end + 1] == '\n') {
                start = end + 2;
            } else {
                start = end + 1;
            }
        } else {
            start = n;
        }
    }
    return lines;
}

std::vector<std::string_view> tokenize(std::string_view line) {
    std::vector<std::string_view> tokens;
    size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
        if (i >= line.size()) break;
        size_t start = i;
        while (i < line.size() && line[i] != ' ' && line[i] != '\t') {
            ++i;
        }
        tokens.push_back(line.substr(start, i - start));
    }
    return tokens;
}

std::vector<Command> parse_input(std::string_view input_text) {
    if (input_text.size() >= 3 &&
        static_cast<unsigned char>(input_text[0]) == 0xEF &&
        static_cast<unsigned char>(input_text[1]) == 0xBB &&
        static_cast<unsigned char>(input_text[2]) == 0xBF) {
        input_text.remove_prefix(3);
    }
    std::vector<std::string_view> lines = split_lines(input_text);
    std::vector<Command> commands;

    size_t line_idx = 0;
    size_t num_lines = lines.size();
    bool orphaned_map_skip = false;

    while (line_idx < num_lines) {
        std::string_view raw_line = lines[line_idx];
        std::string_view trimmed_line = trim_trailing(raw_line);

        if (trimmed_line.empty()) {
            // Blank lines between commands are ignored silently
            line_idx++;
            continue;
        }

        // Check if this line is map-like (consists only of E, P, ., #)
        if (is_map_like_line(raw_line)) {
            if (orphaned_map_skip) {
                // Silently swallow orphaned map lines from a failed create
                line_idx++;
                continue;
            } else {
                // Unexpected stray map data: emit ONE error per contiguous run
                Command stray_cmd;
                stray_cmd.type = CommandType::UNEXPECTED_MAP_DATA;
                commands.push_back(std::move(stray_cmd));
                while (line_idx < num_lines && is_map_like_line(lines[line_idx])) {
                    line_idx++;
                }
                continue;
            }
        }

        // Non-map-like line reached: clear orphan swallowing
        orphaned_map_skip = false;

        std::vector<std::string_view> tokens = tokenize(raw_line);
        if (tokens.empty()) {
            line_idx++;
            continue;
        }

        std::string_view cmd_name = tokens[0];

        if (cmd_name == "create_parking_lot") {
            // 3.a Argument count == 2 and both numbers valid
            if (tokens.size() != 3) {
                Command cmd;
                cmd.type = CommandType::PRE_FAILED;
                cmd.raw_name = "create_parking_lot";
                cmd.error_reason = ErrorReason::INVALID_ARGUMENTS;
                commands.push_back(std::move(cmd));
                orphaned_map_skip = true;
                line_idx++;
                continue;
            }

            uint64_t rows_val = 0;
            uint64_t cols_val = 0;
            auto res_r = parse_positive_number_saturating(tokens[1], 20000000ULL, rows_val);
            auto res_c = parse_positive_number_saturating(tokens[2], 20000000ULL, cols_val);
            if (res_r != ParseNumResult::VALID || res_c != ParseNumResult::VALID) {
                Command cmd;
                cmd.type = CommandType::PRE_FAILED;
                cmd.raw_name = "create_parking_lot";
                cmd.error_reason = ErrorReason::INVALID_ARGUMENTS;
                commands.push_back(std::move(cmd));
                orphaned_map_skip = true;
                line_idx++;
                continue;
            }

            // 3.b rows*cols (64-bit) <= 20,000,000, else grid too large
            if (rows_val > 20000000ULL / cols_val) {
                Command cmd;
                cmd.type = CommandType::PRE_FAILED;
                cmd.raw_name = "create_parking_lot";
                cmd.error_reason = ErrorReason::GRID_TOO_LARGE;
                commands.push_back(std::move(cmd));
                orphaned_map_skip = true;
                line_idx++;
                continue;
            }

            // 3.c & 3.d Consume map lines one at a time up to rows lines
            int64_t rows = static_cast<int64_t>(rows_val);
            int64_t cols = static_cast<int64_t>(cols_val);

            bool map_valid = true;
            int e_count = 0;
            std::vector<std::string> map_lines;
            map_lines.reserve(static_cast<size_t>(rows));

            int64_t r = 0;
            for (; r < rows; ++r) {
                size_t next_idx = line_idx + 1;
                if (next_idx >= num_lines) {
                    // Early EOF
                    map_valid = false;
                    break;
                }
                std::string_view cand_line = lines[next_idx];
                auto cand_tokens = tokenize(cand_line);
                if (!cand_tokens.empty() && is_known_command(cand_tokens[0])) {
                    // Early stop: line is a known command! End block without consuming
                    map_valid = false;
                    break;
                }

                // Consume this row
                line_idx++;
                std::string_view row_trimmed = trim_trailing(cand_line);

                // Row validation: length must equal cols exactly; characters in E, P, ., #
                if (map_valid) {
                    if (static_cast<int64_t>(row_trimmed.size()) != cols) {
                        map_valid = false;
                    } else {
                        for (char ch : row_trimmed) {
                            if (ch == 'E') {
                                e_count++;
                            } else if (ch != 'P' && ch != '.' && ch != '#') {
                                map_valid = false;
                                break;
                            }
                        }
                    }
                    if (map_valid) {
                        map_lines.emplace_back(row_trimmed);
                    }
                }
            }

            // Check if exactly rows lines were read and exactly one 'E'
            if (r != rows || e_count != 1) {
                map_valid = false;
            }

            Command cmd;
            cmd.type = CommandType::CREATE_PARKING_LOT;
            cmd.raw_name = "create_parking_lot";
            cmd.rows = rows;
            cmd.cols = cols;
            cmd.map_is_valid = map_valid;
            if (map_valid) {
                cmd.map_lines = std::move(map_lines);
                orphaned_map_skip = false;
            } else {
                cmd.error_reason = ErrorReason::INVALID_MAP;
                orphaned_map_skip = true;
            }
            commands.push_back(std::move(cmd));
            line_idx++;
            continue;
        }

        // Process other commands
        bool tokens_valid_len = true;
        for (const auto& tok : tokens) {
            if (tok.size() > 256) {
                tokens_valid_len = false;
                break;
            }
        }

        Command cmd;
        cmd.raw_name = std::string(cmd_name);
        for (size_t t = 1; t < tokens.size(); ++t) {
            cmd.args.emplace_back(tokens[t]);
        }

        if (!tokens_valid_len) {
            if (is_known_command(cmd_name)) {
                cmd.type = CommandType::PRE_FAILED;
                cmd.error_reason = ErrorReason::INVALID_ARGUMENTS;
            } else {
                cmd.type = CommandType::UNKNOWN_COMMAND;
            }
            commands.push_back(std::move(cmd));
            line_idx++;
            continue;
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

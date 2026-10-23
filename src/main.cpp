#define _CRT_SECURE_NO_WARNINGS
#include "types.h"
#include "parser.h"
#include "commands.h"
#include "formatter.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <algorithm>

namespace fs = std::filesystem;

namespace {

bool is_unsupported_encoding(std::string_view content) {
    if (content.size() >= 4) {
        // UTF-32 LE: FF FE 00 00
        if (static_cast<unsigned char>(content[0]) == 0xFF &&
            static_cast<unsigned char>(content[1]) == 0xFE &&
            static_cast<unsigned char>(content[2]) == 0x00 &&
            static_cast<unsigned char>(content[3]) == 0x00) {
            return true;
        }
        // UTF-32 BE: 00 00 FE FF
        if (static_cast<unsigned char>(content[0]) == 0x00 &&
            static_cast<unsigned char>(content[1]) == 0x00 &&
            static_cast<unsigned char>(content[2]) == 0xFE &&
            static_cast<unsigned char>(content[3]) == 0xFF) {
            return true;
        }
    }
    if (content.size() >= 2) {
        // UTF-16 LE: FF FE
        if (static_cast<unsigned char>(content[0]) == 0xFF &&
            static_cast<unsigned char>(content[1]) == 0xFE) {
            return true;
        }
        // UTF-16 BE: FE FF
        if (static_cast<unsigned char>(content[0]) == 0xFE &&
            static_cast<unsigned char>(content[1]) == 0xFF) {
            return true;
        }
    }
    // Heuristic: if more than 10% of the first 4096 bytes are NUL
    size_t check_len = std::min(content.size(), static_cast<size_t>(4096));
    if (check_len > 0) {
        size_t nul_count = 0;
        for (size_t i = 0; i < check_len; ++i) {
            if (content[i] == '\0') {
                nul_count++;
            }
        }
        if (nul_count * 10 > check_len) {
            return true;
        }
    }
    return false;
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    std::string out_buffer;
    out_buffer.reserve(65536);

    try {
        if (argc < 2) {
            std::fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
            return 1;
        }

        const char* file_path = argv[1];
        std::error_code ec;
        fs::file_status st = fs::status(file_path, ec);
        if (ec || !fs::exists(st)) {
            std::fprintf(stderr, "Error: file '%s' does not exist or cannot be accessed\n", file_path);
            return 1;
        }
        if (fs::is_directory(st)) {
            std::fprintf(stderr, "Error: '%s' is a directory\n", file_path);
            return 1;
        }

        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            std::fprintf(stderr, "Error: cannot open file '%s'\n", file_path);
            return 1;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        if (is_unsupported_encoding(content)) {
            std::fprintf(stderr, "Error: unsupported file encoding\n");
            return 1;
        }

        std::string_view view = content;
        if (view.size() >= 3 &&
            static_cast<unsigned char>(view[0]) == 0xEF &&
            static_cast<unsigned char>(view[1]) == 0xBB &&
            static_cast<unsigned char>(view[2]) == 0xBF) {
            view.remove_prefix(3);
        }

        std::vector<parking::Command> cmds = parking::parser::parse_input(view);

        parking::LotState lot;

        static constexpr size_t FLUSH_THRESHOLD = 32 * 1024 * 1024;
        for (const auto& cmd : cmds) {
            parking::CommandResult res = parking::commands::execute(lot, cmd);
            parking::formatter::format_result(res, out_buffer);

            if (out_buffer.size() >= FLUSH_THRESHOLD) {
                size_t written = std::fwrite(out_buffer.data(), 1, out_buffer.size(), stdout);
                if (written != out_buffer.size()) {
                    std::fprintf(stderr, "Error: failed to write output\n");
                    return 1;
                }
                out_buffer.clear();
            }
        }

        if (!out_buffer.empty()) {
            size_t written = std::fwrite(out_buffer.data(), 1, out_buffer.size(), stdout);
            if (written != out_buffer.size()) {
                std::fprintf(stderr, "Error: failed to write output\n");
                return 1;
            }
            out_buffer.clear();
        }

        return 0;
    } catch (const std::exception&) {
        if (!out_buffer.empty()) {
            std::fwrite(out_buffer.data(), 1, out_buffer.size(), stdout);
        }
        std::fprintf(stderr, "Fatal: internal error\n");
        return 2;
    } catch (...) {
        if (!out_buffer.empty()) {
            std::fwrite(out_buffer.data(), 1, out_buffer.size(), stdout);
        }
        std::fprintf(stderr, "Fatal: internal error\n");
        return 2;
    }
}

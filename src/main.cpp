#define _CRT_SECURE_NO_WARNINGS
#include "types.h"
#include "parser.h"
#include "commands.h"
#include "formatter.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    // Extra command-line arguments are ignored as permitted by specification (Rule 9).
    const char* file_path = argv[1];
    std::error_code ec;
    fs::file_status status = fs::status(file_path, ec);
    if (ec || !fs::exists(status)) {
        std::fprintf(stderr, "Error: file '%s' does not exist or cannot be accessed\n", file_path);
        return 1;
    }
    if (fs::is_directory(status)) {
        std::fprintf(stderr, "Error: '%s' is a directory\n", file_path);
        return 1;
    }

    FILE* fp = std::fopen(file_path, "rb");
    if (!fp) {
        std::fprintf(stderr, "Error: cannot open file '%s'\n", file_path);
        return 1;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        std::fclose(fp);
        std::fprintf(stderr, "Error: failed to seek file '%s'\n", file_path);
        return 1;
    }
    long file_size = std::ftell(fp);
    if (file_size < 0) {
        std::fclose(fp);
        std::fprintf(stderr, "Error: failed to get size of file '%s'\n", file_path);
        return 1;
    }
    std::fseek(fp, 0, SEEK_SET);

    std::string content;
    if (file_size > 0) {
        content.resize(static_cast<size_t>(file_size));
        size_t bytes_read = std::fread(&content[0], 1, static_cast<size_t>(file_size), fp);
        content.resize(bytes_read);
    }
    std::fclose(fp);

    std::vector<parking::Command> cmds = parking::parser::parse_input(content);

    parking::LotState lot;
    std::string out_buffer;
    out_buffer.reserve(65536);

    for (const auto& cmd : cmds) {
        parking::CommandResult res = parking::commands::execute(lot, cmd);
        parking::formatter::format_result(res, out_buffer);
    }

    if (!out_buffer.empty()) {
        std::fwrite(out_buffer.data(), 1, out_buffer.size(), stdout);
    }

    return 0;
}

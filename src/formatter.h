#pragma once

#include "types.h"
#include <string>
#include <string_view>

namespace parking::formatter {

// Converts ErrorReason enum to exact specification string
const char* reason_to_string(ErrorReason reason);

// Sanitizes unknown command name: printable ASCII only, other bytes '?', max 32 chars then "..."
std::string sanitize_command_name(std::string_view name);

// Formats a CommandResult into output text (ending with '\n').
// This is the ONLY place in the codebase that builds output text.
void format_result(const CommandResult& res, std::string& out_buffer);

} // namespace parking::formatter

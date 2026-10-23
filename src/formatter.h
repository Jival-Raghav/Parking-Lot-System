#pragma once

#include "types.h"
#include <string>

namespace parking::formatter {

// Formats a CommandResult into output text (ending with '\n').
// This is the ONLY place in the codebase that builds output text.
void format_result(const CommandResult& res, std::string& out_buffer);

} // namespace parking::formatter

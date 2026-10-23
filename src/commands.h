#pragma once

#include "types.h"

namespace parking::commands {

// Executes any command against lot state and returns a CommandResult.
// Core logic does NO I/O (no printing, no file access).
CommandResult execute(LotState& lot, const Command& cmd);

// Individual command handlers
CommandResult execute_create_parking_lot(LotState& lot, const Command& cmd);
CommandResult execute_park(LotState& lot, const Command& cmd);
CommandResult execute_leave(LotState& lot, const Command& cmd);
CommandResult execute_status(const LotState& lot, const Command& cmd);
CommandResult execute_reg_nums_for_colour(const LotState& lot, const Command& cmd);
CommandResult execute_slot_nums_for_colour(const LotState& lot, const Command& cmd);
CommandResult execute_slot_for_reg(const LotState& lot, const Command& cmd);

} // namespace parking::commands

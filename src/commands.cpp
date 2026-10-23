#include "commands.h"
#include "parser.h"
#include "grid.h"
#include "distance.h"
#include "slots.h"
#include "state.h"

namespace parking::commands {

CommandResult execute_create_parking_lot(LotState& lot, const Command& cmd) {
    if (cmd.type == CommandType::PRE_FAILED) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "create_parking_lot";
        res.reason = cmd.error_reason;
        return res;
    }

    if (lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "create_parking_lot";
        res.reason = ErrorReason::LOT_ALREADY_EXISTS;
        return res;
    }

    if (!cmd.map_is_valid) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "create_parking_lot";
        res.reason = ErrorReason::INVALID_MAP;
        return res;
    }

    int32_t entrance_idx = -1;
    if (!grid::validate_map(static_cast<int32_t>(cmd.rows), static_cast<int32_t>(cmd.cols), cmd.map_lines, entrance_idx)) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "create_parking_lot";
        res.reason = ErrorReason::INVALID_MAP;
        return res;
    }

    // Build new lot locally first
    LotState new_lot;
    new_lot.rows = static_cast<int32_t>(cmd.rows);
    new_lot.cols = static_cast<int32_t>(cmd.cols);
    new_lot.total_cells = static_cast<size_t>(new_lot.rows) * static_cast<size_t>(new_lot.cols);
    
    new_lot.grid.resize(new_lot.total_cells);
    for (int32_t r = 0; r < new_lot.rows; ++r) {
        const std::string& line = cmd.map_lines[static_cast<size_t>(r)];
        for (int32_t c = 0; c < new_lot.cols; ++c) {
            new_lot.grid[static_cast<size_t>(r) * static_cast<size_t>(new_lot.cols) + static_cast<size_t>(c)] = line[static_cast<size_t>(c)];
        }
    }

    std::vector<int32_t> road_dist = distance::compute_road_distances(
        new_lot.rows, new_lot.cols, new_lot.grid, entrance_idx
    );

    slots::build_usable_slots(new_lot.rows, new_lot.cols, new_lot.grid, road_dist, new_lot);
    new_lot.created = true;

    lot = std::move(new_lot);

    CommandResult res;
    res.kind = ResultKind::CREATE_SUCCESS;
    res.usable_slots_count = lot.usable_count;
    return res;
}

CommandResult execute_park(LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "park";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "park";
        res.reason = cmd.error_reason;
        return res;
    }
    if (cmd.args.size() != 2 ||
        !parser::is_valid_string_token(cmd.args[0]) ||
        !parser::is_valid_string_token(cmd.args[1])) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "park";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    const std::string& reg = cmd.args[0];
    const std::string& colour = cmd.args[1];

    if (state::is_registration_parked(lot, reg)) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "park";
        res.reason = ErrorReason::REGISTRATION_ALREADY_PARKED;
        return res;
    }

    if (lot.free_ranks.empty()) {
        CommandResult res;
        res.kind = ResultKind::LOT_FULL;
        return res;
    }

    Position pos = state::park_car(lot, reg, colour);
    CommandResult res;
    res.kind = ResultKind::ALLOCATED_SLOT;
    res.slot = pos;
    return res;
}

CommandResult execute_leave(LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = cmd.error_reason;
        return res;
    }
    if (cmd.args.size() != 2) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    uint64_t r_val = 0;
    uint64_t c_val = 0;
    auto r_res = parser::parse_positive_number_saturating(cmd.args[0], 20000000ULL, r_val);
    auto c_res = parser::parse_positive_number_saturating(cmd.args[1], 20000000ULL, c_val);

    if (r_res != parser::ParseNumResult::VALID || c_res != parser::ParseNumResult::VALID) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    if (r_val > static_cast<uint64_t>(lot.rows) || c_val > static_cast<uint64_t>(lot.cols)) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::SLOT_OUT_OF_RANGE;
        return res;
    }

    int32_t r = static_cast<int32_t>(r_val);
    int32_t c = static_cast<int32_t>(c_val);
    int32_t cell_idx = static_cast<int32_t>(grid::to_cell_index(r, c, lot.cols));

    if (!state::is_usable_slot(lot, cell_idx)) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::NOT_A_USABLE_SLOT;
        return res;
    }

    if (!state::is_slot_occupied(lot, cell_idx)) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "leave";
        res.reason = ErrorReason::SLOT_ALREADY_FREE;
        return res;
    }

    state::free_slot(lot, cell_idx);
    CommandResult res;
    res.kind = ResultKind::SLOT_IS_FREE;
    res.slot = Position{r, c};
    return res;
}

CommandResult execute_status(const LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "status";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED || !cmd.args.empty()) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "status";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    if (lot.all_occupied.empty()) {
        CommandResult res;
        res.kind = ResultKind::EMPTY_LOT;
        return res;
    }

    CommandResult res;
    res.kind = ResultKind::STATUS_TABLE;
    res.status_rows.reserve(lot.all_occupied.size());

    for (int32_t cell_idx : lot.all_occupied) {
        Position pos = grid::to_position(static_cast<size_t>(cell_idx), lot.cols);
        int32_t rank = lot.cell_to_rank[static_cast<size_t>(cell_idx)];
        const Car& car = *(lot.rank_occupant[static_cast<size_t>(rank)]);
        res.status_rows.push_back(OccupiedSlotInfo{pos, car.registration, car.colour});
    }

    return res;
}

CommandResult execute_reg_nums_for_colour(const LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "registration_numbers_for_cars_with_colour";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED || cmd.args.size() != 1 || !parser::is_valid_string_token(cmd.args[0])) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "registration_numbers_for_cars_with_colour";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    std::string colour_lower = state::to_lower_ascii(cmd.args[0]);
    auto it = lot.colour_to_cells.find(colour_lower);
    if (it == lot.colour_to_cells.end() || it->second.empty()) {
        CommandResult res;
        res.kind = ResultKind::NOT_FOUND;
        return res;
    }

    CommandResult res;
    res.kind = ResultKind::REGISTRATION_LIST;
    res.strings.reserve(it->second.size());
    for (int32_t cell_idx : it->second) {
        int32_t rank = lot.cell_to_rank[static_cast<size_t>(cell_idx)];
        const Car& car = *(lot.rank_occupant[static_cast<size_t>(rank)]);
        res.strings.push_back(car.registration);
    }
    return res;
}

CommandResult execute_slot_nums_for_colour(const LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "slot_numbers_for_cars_with_colour";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED || cmd.args.size() != 1 || !parser::is_valid_string_token(cmd.args[0])) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "slot_numbers_for_cars_with_colour";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    std::string colour_lower = state::to_lower_ascii(cmd.args[0]);
    auto it = lot.colour_to_cells.find(colour_lower);
    if (it == lot.colour_to_cells.end() || it->second.empty()) {
        CommandResult res;
        res.kind = ResultKind::NOT_FOUND;
        return res;
    }

    CommandResult res;
    res.kind = ResultKind::SLOT_LIST;
    res.slots.reserve(it->second.size());
    for (int32_t cell_idx : it->second) {
        Position pos = grid::to_position(static_cast<size_t>(cell_idx), lot.cols);
        res.slots.push_back(pos);
    }
    return res;
}

CommandResult execute_slot_for_reg(const LotState& lot, const Command& cmd) {
    if (!lot.created) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "slot_number_for_registration_number";
        res.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
        return res;
    }
    if (cmd.type == CommandType::PRE_FAILED || cmd.args.size() != 1 || !parser::is_valid_string_token(cmd.args[0])) {
        CommandResult res;
        res.kind = ResultKind::COMMAND_FAILED;
        res.command_name = "slot_number_for_registration_number";
        res.reason = ErrorReason::INVALID_ARGUMENTS;
        return res;
    }

    const std::string& reg = cmd.args[0];
    auto it = lot.reg_to_cell.find(reg);
    if (it == lot.reg_to_cell.end()) {
        CommandResult res;
        res.kind = ResultKind::NOT_FOUND;
        return res;
    }

    Position pos = grid::to_position(static_cast<size_t>(it->second), lot.cols);
    CommandResult res;
    res.kind = ResultKind::SINGLE_SLOT;
    res.slot = pos;
    return res;
}

CommandResult execute(LotState& lot, const Command& cmd) {
    switch (cmd.type) {
        case CommandType::CREATE_PARKING_LOT:
            return execute_create_parking_lot(lot, cmd);

        case CommandType::PARK:
            return execute_park(lot, cmd);

        case CommandType::LEAVE:
            return execute_leave(lot, cmd);

        case CommandType::STATUS:
            return execute_status(lot, cmd);

        case CommandType::REG_NUMS_FOR_COLOUR:
            return execute_reg_nums_for_colour(lot, cmd);

        case CommandType::SLOT_NUMS_FOR_COLOUR:
            return execute_slot_nums_for_colour(lot, cmd);

        case CommandType::SLOT_FOR_REG:
            return execute_slot_for_reg(lot, cmd);

        case CommandType::PRE_FAILED: {
            CommandResult res;
            res.kind = ResultKind::COMMAND_FAILED;
            res.command_name = cmd.raw_name;
            res.reason = cmd.error_reason;
            return res;
        }

        case CommandType::UNEXPECTED_MAP_DATA: {
            CommandResult res;
            res.kind = ResultKind::ERROR_UNEXPECTED_MAP_DATA;
            return res;
        }

        case CommandType::UNKNOWN_COMMAND:
        default: {
            CommandResult res;
            res.kind = ResultKind::ERROR_UNKNOWN_COMMAND;
            res.raw_unknown_name = cmd.raw_name;
            return res;
        }
    }
}

} // namespace parking::commands

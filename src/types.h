#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <optional>

namespace parking {

// 1-based grid coordinates
struct Position {
    int32_t row{0}; // 1-based
    int32_t col{0}; // 1-based

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator<(const Position& other) const {
        if (row != other.row) return row < other.row;
        return col < other.col;
    }
};

// Information about a usable parking slot
struct UsableSlot {
    int32_t distance{0};
    int32_t row{0};       // 1-based
    int32_t col{0};       // 1-based
    int32_t cell_index{0}; // 0-based flat grid index
};

// Car information
struct Car {
    std::string registration;
    std::string colour; // original colour string as entered
};

// Command representation from parsing
enum class CommandType {
    CREATE_PARKING_LOT,
    PARK,
    LEAVE,
    STATUS,
    REG_NUMS_FOR_COLOUR,
    SLOT_NUMS_FOR_COLOUR,
    SLOT_FOR_REG,
    UNKNOWN_COMMAND,
    INVALID_ARGS,
    INVALID_DIMENSIONS,
    INVALID_MAP,
    GRID_TOO_LARGE
};

struct Command {
    CommandType type{CommandType::UNKNOWN_COMMAND};
    std::string raw_name; // original command name token
    std::vector<std::string> args;
    
    // For CREATE_PARKING_LOT:
    int64_t rows{0};
    int64_t cols{0};
    std::vector<std::string> map_lines;
    
    // For parsing errors detected early:
    std::string error_detail;
};

// Result kinds for commands
enum class ResultKind {
    CREATE_SUCCESS,
    ALLOCATED_SLOT,
    SLOT_IS_FREE,
    STATUS_TABLE,
    EMPTY_LOT,
    REGISTRATION_LIST,
    SLOT_LIST,
    SINGLE_SLOT,
    NOT_FOUND,
    LOT_FULL,
    ERROR_NOT_CREATED,
    ERROR_ALREADY_CREATED,
    ERROR_UNKNOWN_COMMAND,
    ERROR_INVALID_ARGUMENTS,
    ERROR_INVALID_DIMENSIONS,
    ERROR_INVALID_MAP,
    ERROR_GRID_TOO_LARGE,
    ERROR_REGISTRATION_ALREADY_PARKED,
    ERROR_SLOT_OUT_OF_RANGE,
    ERROR_NOT_USABLE_SLOT,
    ERROR_SLOT_ALREADY_FREE
};

struct OccupiedSlotInfo {
    Position pos;
    std::string registration;
    std::string colour;
};

struct CommandResult {
    ResultKind kind{ResultKind::ERROR_UNKNOWN_COMMAND};
    std::string command_name; // for invalid arguments and unknown command
    Position slot{0, 0};      // for allocated slot, slot is free, slot errors, single slot
    std::string registration; // for registration already parked
    int32_t usable_slots_count{0}; // for create_parking_lot
    std::vector<OccupiedSlotInfo> status_rows; // for status table
    std::vector<std::string> strings; // for comma-separated reg numbers
    std::vector<Position> slots;      // for comma-separated slot positions
};

// All state lives in LotState, passed by reference to functions
struct LotState {
    bool created{false};
    int32_t rows{0};
    int32_t cols{0};
    size_t total_cells{0};
    
    // Grid: flat vector of characters (index = (r-1)*cols + (c-1))
    std::vector<char> grid;
    
    // Usable slots and ranks:
    int32_t usable_count{0};
    std::vector<UsableSlot> usable_slots; // sorted by (distance, row, col)
    std::vector<int32_t> rank_to_cell;    // rank -> flat cell index
    std::vector<int32_t> cell_to_rank;    // flat cell index -> rank (-1 if not usable)
    
    // Free slots heap: min-heap of free ranks (smallest rank = nearest/best slot)
    std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t>> free_ranks;
    
    // Occupancy:
    // rank -> optional<Car>
    std::vector<std::optional<Car>> rank_occupant;
    
    // Indexes:
    // reg_to_cell: exact registration -> cell_index (case-sensitive)
    std::unordered_map<std::string, int32_t> reg_to_cell;
    // colour_to_cells: ASCII lowercased colour -> set of cell_indices (row-major order)
    std::unordered_map<std::string, std::set<int32_t>> colour_to_cells;
    // all_occupied: set of cell_indices of all occupied slots (row-major order)
    std::set<int32_t> all_occupied;
};

} // namespace parking

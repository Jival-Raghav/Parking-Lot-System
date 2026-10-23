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

// Specific error reasons per requirement specification
enum class ErrorReason {
    INVALID_ARGUMENTS,           // "invalid arguments"
    PARKING_LOT_NOT_CREATED,     // "parking lot not created"
    LOT_ALREADY_EXISTS,          // "lot already exists"
    GRID_TOO_LARGE,              // "grid too large"
    INVALID_MAP,                 // "invalid map"
    REGISTRATION_ALREADY_PARKED, // "registration already parked"
    SLOT_OUT_OF_RANGE,           // "slot out of range"
    NOT_A_USABLE_SLOT,           // "not a usable slot"
    SLOT_ALREADY_FREE            // "slot already free"
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
    UNEXPECTED_MAP_DATA,
    PRE_FAILED // Command that failed during parsing/pre-validation
};

struct Command {
    CommandType type{CommandType::UNKNOWN_COMMAND};
    std::string raw_name; // original command name token
    std::vector<std::string> args;
    
    // For CREATE_PARKING_LOT:
    int64_t rows{0};
    int64_t cols{0};
    std::vector<std::string> map_lines;
    bool map_is_valid{true};
    
    // Error reason if pre-failed
    ErrorReason error_reason{ErrorReason::INVALID_ARGUMENTS};
};

// Result kinds for command outputs
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
    COMMAND_FAILED,               // "Error: <command_name> failed: <reason>"
    ERROR_UNKNOWN_COMMAND,        // "Error: unknown command '<sanitized_name>'"
    ERROR_UNEXPECTED_MAP_DATA     // "Error: unexpected map data ignored"
};

struct OccupiedSlotInfo {
    Position pos;
    std::string registration;
    std::string colour;
};

struct CommandResult {
    ResultKind kind{ResultKind::COMMAND_FAILED};
    std::string command_name; // for "Error: <command_name> failed: <reason>"
    ErrorReason reason{ErrorReason::INVALID_ARGUMENTS};
    Position slot{0, 0};      // for allocated slot, slot is free, single slot
    int32_t usable_slots_count{0}; // for create_parking_lot
    std::vector<OccupiedSlotInfo> status_rows; // for status table
    std::vector<std::string> strings; // for comma-separated reg numbers
    std::vector<Position> slots;      // for comma-separated slot positions
    std::string raw_unknown_name;     // for sanitized unknown command printing
};

// All lot state lives in LotState, passed by reference to functions
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

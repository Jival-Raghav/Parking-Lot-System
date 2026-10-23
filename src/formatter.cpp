#include "formatter.h"

namespace parking::formatter {

void format_result(const CommandResult& res, std::string& out_buffer) {
    switch (res.kind) {
        case ResultKind::CREATE_SUCCESS:
            out_buffer += "Created a parking lot with " + std::to_string(res.usable_slots_count) + " usable slots\n";
            break;

        case ResultKind::ALLOCATED_SLOT:
            out_buffer += "Allocated slot: (" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ")\n";
            break;

        case ResultKind::SLOT_IS_FREE:
            out_buffer += "Slot (" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ") is free\n";
            break;

        case ResultKind::STATUS_TABLE: {
            out_buffer += "Slot     Registration No    Colour\n";
            for (const auto& row : res.status_rows) {
                std::string slot_str = "(" + std::to_string(row.pos.row) + ", " + std::to_string(row.pos.col) + ")";
                size_t pad1 = (slot_str.size() < 9) ? (9 - slot_str.size()) : 1;
                size_t pad2 = (row.registration.size() < 19) ? (19 - row.registration.size()) : 1;
                
                out_buffer += slot_str;
                out_buffer.append(pad1, ' ');
                out_buffer += row.registration;
                out_buffer.append(pad2, ' ');
                out_buffer += row.colour;
                out_buffer += '\n';
            }
            break;
        }

        case ResultKind::EMPTY_LOT:
            out_buffer += "Parking lot is empty\n";
            break;

        case ResultKind::REGISTRATION_LIST: {
            for (size_t i = 0; i < res.strings.size(); ++i) {
                if (i > 0) out_buffer += ", ";
                out_buffer += res.strings[i];
            }
            out_buffer += '\n';
            break;
        }

        case ResultKind::SLOT_LIST: {
            for (size_t i = 0; i < res.slots.size(); ++i) {
                if (i > 0) out_buffer += ", ";
                out_buffer += "(" + std::to_string(res.slots[i].row) + ", " + std::to_string(res.slots[i].col) + ")";
            }
            out_buffer += '\n';
            break;
        }

        case ResultKind::SINGLE_SLOT:
            out_buffer += "(" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ")\n";
            break;

        case ResultKind::NOT_FOUND:
            out_buffer += "Not found\n";
            break;

        case ResultKind::LOT_FULL:
            out_buffer += "Sorry, parking lot is full\n";
            break;

        case ResultKind::ERROR_NOT_CREATED:
            out_buffer += "Error: parking lot not created\n";
            break;

        case ResultKind::ERROR_ALREADY_CREATED:
            out_buffer += "Error: parking lot already created\n";
            break;

        case ResultKind::ERROR_UNKNOWN_COMMAND:
            out_buffer += "Error: unknown command '" + res.command_name + "'\n";
            break;

        case ResultKind::ERROR_INVALID_ARGUMENTS:
            out_buffer += "Error: invalid arguments for " + res.command_name + "\n";
            break;

        case ResultKind::ERROR_INVALID_DIMENSIONS:
            out_buffer += "Error: invalid dimensions\n";
            break;

        case ResultKind::ERROR_INVALID_MAP:
            out_buffer += "Error: invalid map\n";
            break;

        case ResultKind::ERROR_GRID_TOO_LARGE:
            out_buffer += "Error: grid too large\n";
            break;

        case ResultKind::ERROR_REGISTRATION_ALREADY_PARKED:
            out_buffer += "Error: registration " + res.registration + " is already parked\n";
            break;

        case ResultKind::ERROR_SLOT_OUT_OF_RANGE:
            out_buffer += "Error: slot (" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ") is out of range\n";
            break;

        case ResultKind::ERROR_NOT_USABLE_SLOT:
            out_buffer += "Error: (" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ") is not a usable parking slot\n";
            break;

        case ResultKind::ERROR_SLOT_ALREADY_FREE:
            out_buffer += "Error: slot (" + std::to_string(res.slot.row) + ", " +
                          std::to_string(res.slot.col) + ") is already free\n";
            break;
    }
}

} // namespace parking::formatter

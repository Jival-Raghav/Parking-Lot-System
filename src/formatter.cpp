#include "formatter.h"
#include <algorithm>

namespace parking::formatter {

const char* reason_to_string(ErrorReason reason) {
    switch (reason) {
        case ErrorReason::INVALID_ARGUMENTS:
            return "invalid arguments";
        case ErrorReason::PARKING_LOT_NOT_CREATED:
            return "parking lot not created";
        case ErrorReason::LOT_ALREADY_EXISTS:
            return "lot already exists";
        case ErrorReason::GRID_TOO_LARGE:
            return "grid too large";
        case ErrorReason::INVALID_MAP:
            return "invalid map";
        case ErrorReason::REGISTRATION_ALREADY_PARKED:
            return "registration already parked";
        case ErrorReason::SLOT_OUT_OF_RANGE:
            return "slot out of range";
        case ErrorReason::NOT_A_USABLE_SLOT:
            return "not a usable slot";
        case ErrorReason::SLOT_ALREADY_FREE:
            return "slot already free";
    }
    return "unknown error";
}

std::string sanitize_command_name(std::string_view name) {
    std::string sanitized;
    size_t limit = std::min(name.size(), static_cast<size_t>(32));
    sanitized.reserve(36);

    for (size_t i = 0; i < limit; ++i) {
        unsigned char uc = static_cast<unsigned char>(name[i]);
        if (uc >= 0x20 && uc <= 0x7E) {
            sanitized.push_back(static_cast<char>(uc));
        } else {
            sanitized.push_back('?');
        }
    }

    if (name.size() > 32) {
        sanitized += "...";
    }

    return sanitized;
}

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

        case ResultKind::COMMAND_FAILED:
            out_buffer += "Error: " + res.command_name + " failed: " + reason_to_string(res.reason) + "\n";
            break;

        case ResultKind::ERROR_UNKNOWN_COMMAND:
            out_buffer += "Error: unknown command '" + sanitize_command_name(res.raw_unknown_name) + "'\n";
            break;

        case ResultKind::ERROR_UNEXPECTED_MAP_DATA:
            out_buffer += "Error: unexpected map data ignored\n";
            break;
    }
}

} // namespace parking::formatter

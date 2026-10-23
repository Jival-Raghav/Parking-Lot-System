#include "test_framework.h"
#include "../src/formatter.h"

using namespace parking;

TEST_CASE(test_format_success_outputs) {
    std::string out;

    // Creation
    CommandResult r_create;
    r_create.kind = ResultKind::CREATE_SUCCESS;
    r_create.usable_slots_count = 10;
    formatter::format_result(r_create, out);
    CHECK_EQ(out, "Created a parking lot with 10 usable slots\n");

    // Allocated slot
    out.clear();
    CommandResult r_alloc;
    r_alloc.kind = ResultKind::ALLOCATED_SLOT;
    r_alloc.slot = {2, 1};
    formatter::format_result(r_alloc, out);
    CHECK_EQ(out, "Allocated slot: (2, 1)\n");

    // Slot free
    out.clear();
    CommandResult r_free;
    r_free.kind = ResultKind::SLOT_IS_FREE;
    r_free.slot = {1, 7};
    formatter::format_result(r_free, out);
    CHECK_EQ(out, "Slot (1, 7) is free\n");

    // Empty lot
    out.clear();
    CommandResult r_empty;
    r_empty.kind = ResultKind::EMPTY_LOT;
    formatter::format_result(r_empty, out);
    CHECK_EQ(out, "Parking lot is empty\n");

    // Status table exact spacing from PDF
    out.clear();
    CommandResult r_stat;
    r_stat.kind = ResultKind::STATUS_TABLE;
    r_stat.status_rows = {
        {{2, 3}, "KA-01-HH-9999", "White"},
        {{2, 5}, "KA-01-BB-0001", "Black"}
    };
    formatter::format_result(r_stat, out);
    std::string expected_stat = 
        "Slot     Registration No    Colour\n"
        "(2, 3)   KA-01-HH-9999      White\n"
        "(2, 5)   KA-01-BB-0001      Black\n";
    CHECK_EQ(out, expected_stat);

    // Comma-separated list (single element)
    out.clear();
    CommandResult r_list1;
    r_list1.kind = ResultKind::REGISTRATION_LIST;
    r_list1.strings = {"KA-01-1111"};
    formatter::format_result(r_list1, out);
    CHECK_EQ(out, "KA-01-1111\n");

    // Comma-separated list (multiple elements)
    out.clear();
    CommandResult r_list_multi;
    r_list_multi.kind = ResultKind::REGISTRATION_LIST;
    r_list_multi.strings = {"DL-12-AA-9999", "KA-01-P-333", "KA-01-HH-9999"};
    formatter::format_result(r_list_multi, out);
    CHECK_EQ(out, "DL-12-AA-9999, KA-01-P-333, KA-01-HH-9999\n");

    // Comma-separated slots
    out.clear();
    CommandResult r_slots;
    r_slots.kind = ResultKind::SLOT_LIST;
    r_slots.slots = {{1, 7}, {2, 1}, {2, 3}};
    formatter::format_result(r_slots, out);
    CHECK_EQ(out, "(1, 7), (2, 1), (2, 3)\n");

    // Single slot
    out.clear();
    CommandResult r_single;
    r_single.kind = ResultKind::SINGLE_SLOT;
    r_single.slot = {4, 7};
    formatter::format_result(r_single, out);
    CHECK_EQ(out, "(4, 7)\n");

    // Not found
    out.clear();
    CommandResult r_nf;
    r_nf.kind = ResultKind::NOT_FOUND;
    formatter::format_result(r_nf, out);
    CHECK_EQ(out, "Not found\n");

    // Full lot
    out.clear();
    CommandResult r_full;
    r_full.kind = ResultKind::LOT_FULL;
    formatter::format_result(r_full, out);
    CHECK_EQ(out, "Sorry, parking lot is full\n");
}

TEST_CASE(test_format_error_messages) {
    std::string out;

    // 1. invalid arguments
    CommandResult r1;
    r1.kind = ResultKind::COMMAND_FAILED;
    r1.command_name = "park";
    r1.reason = ErrorReason::INVALID_ARGUMENTS;
    formatter::format_result(r1, out);
    CHECK_EQ(out, "Error: park failed: invalid arguments\n");

    // 2. parking lot not created
    out.clear();
    CommandResult r2;
    r2.kind = ResultKind::COMMAND_FAILED;
    r2.command_name = "status";
    r2.reason = ErrorReason::PARKING_LOT_NOT_CREATED;
    formatter::format_result(r2, out);
    CHECK_EQ(out, "Error: status failed: parking lot not created\n");

    // 3. lot already exists
    out.clear();
    CommandResult r3;
    r3.kind = ResultKind::COMMAND_FAILED;
    r3.command_name = "create_parking_lot";
    r3.reason = ErrorReason::LOT_ALREADY_EXISTS;
    formatter::format_result(r3, out);
    CHECK_EQ(out, "Error: create_parking_lot failed: lot already exists\n");

    // 4. grid too large
    out.clear();
    CommandResult r4;
    r4.kind = ResultKind::COMMAND_FAILED;
    r4.command_name = "create_parking_lot";
    r4.reason = ErrorReason::GRID_TOO_LARGE;
    formatter::format_result(r4, out);
    CHECK_EQ(out, "Error: create_parking_lot failed: grid too large\n");

    // 5. invalid map
    out.clear();
    CommandResult r5;
    r5.kind = ResultKind::COMMAND_FAILED;
    r5.command_name = "create_parking_lot";
    r5.reason = ErrorReason::INVALID_MAP;
    formatter::format_result(r5, out);
    CHECK_EQ(out, "Error: create_parking_lot failed: invalid map\n");

    // 6. registration already parked
    out.clear();
    CommandResult r6;
    r6.kind = ResultKind::COMMAND_FAILED;
    r6.command_name = "park";
    r6.reason = ErrorReason::REGISTRATION_ALREADY_PARKED;
    formatter::format_result(r6, out);
    CHECK_EQ(out, "Error: park failed: registration already parked\n");

    // 7. slot out of range
    out.clear();
    CommandResult r7;
    r7.kind = ResultKind::COMMAND_FAILED;
    r7.command_name = "leave";
    r7.reason = ErrorReason::SLOT_OUT_OF_RANGE;
    formatter::format_result(r7, out);
    CHECK_EQ(out, "Error: leave failed: slot out of range\n");

    // 8. not a usable slot
    out.clear();
    CommandResult r8;
    r8.kind = ResultKind::COMMAND_FAILED;
    r8.command_name = "leave";
    r8.reason = ErrorReason::NOT_A_USABLE_SLOT;
    formatter::format_result(r8, out);
    CHECK_EQ(out, "Error: leave failed: not a usable slot\n");

    // 9. slot already free
    out.clear();
    CommandResult r9;
    r9.kind = ResultKind::COMMAND_FAILED;
    r9.command_name = "leave";
    r9.reason = ErrorReason::SLOT_ALREADY_FREE;
    formatter::format_result(r9, out);
    CHECK_EQ(out, "Error: leave failed: slot already free\n");

    // 10. unknown command (with sanitization)
    out.clear();
    CommandResult r10;
    r10.kind = ResultKind::ERROR_UNKNOWN_COMMAND;
    r10.raw_unknown_name = "foo";
    formatter::format_result(r10, out);
    CHECK_EQ(out, "Error: unknown command 'foo'\n");

    // Sanitization: non-printable replaced by '?'
    out.clear();
    CommandResult r10_np;
    r10_np.kind = ResultKind::ERROR_UNKNOWN_COMMAND;
    r10_np.raw_unknown_name = "f\x01\x1F" "o";
    formatter::format_result(r10_np, out);
    CHECK_EQ(out, "Error: unknown command 'f??o'\n");

    // Sanitization: max 32 chars + "..."
    out.clear();
    CommandResult r10_long;
    r10_long.kind = ResultKind::ERROR_UNKNOWN_COMMAND;
    r10_long.raw_unknown_name = std::string(40, 'a');
    formatter::format_result(r10_long, out);
    CHECK_EQ(out, "Error: unknown command '" + std::string(32, 'a') + "...'\n");

    // 11. unexpected map data ignored
    out.clear();
    CommandResult r11;
    r11.kind = ResultKind::ERROR_UNEXPECTED_MAP_DATA;
    formatter::format_result(r11, out);
    CHECK_EQ(out, "Error: unexpected map data ignored\n");
}

int main() {
    return test_framework::run_all_tests();
}

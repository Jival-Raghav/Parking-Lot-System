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

    // 1. not created
    CommandResult r1; r1.kind = ResultKind::ERROR_NOT_CREATED;
    formatter::format_result(r1, out);
    CHECK_EQ(out, "Error: parking lot not created\n");

    // 2. already created
    out.clear();
    CommandResult r2; r2.kind = ResultKind::ERROR_ALREADY_CREATED;
    formatter::format_result(r2, out);
    CHECK_EQ(out, "Error: parking lot already created\n");

    // 3. unknown command
    out.clear();
    CommandResult r3; r3.kind = ResultKind::ERROR_UNKNOWN_COMMAND; r3.command_name = "foo";
    formatter::format_result(r3, out);
    CHECK_EQ(out, "Error: unknown command 'foo'\n");

    // 4. invalid arguments
    out.clear();
    CommandResult r4; r4.kind = ResultKind::ERROR_INVALID_ARGUMENTS; r4.command_name = "park";
    formatter::format_result(r4, out);
    CHECK_EQ(out, "Error: invalid arguments for park\n");

    // 5. invalid dimensions
    out.clear();
    CommandResult r5; r5.kind = ResultKind::ERROR_INVALID_DIMENSIONS;
    formatter::format_result(r5, out);
    CHECK_EQ(out, "Error: invalid dimensions\n");

    // 6. invalid map
    out.clear();
    CommandResult r6; r6.kind = ResultKind::ERROR_INVALID_MAP;
    formatter::format_result(r6, out);
    CHECK_EQ(out, "Error: invalid map\n");

    // 7. grid too large
    out.clear();
    CommandResult r7; r7.kind = ResultKind::ERROR_GRID_TOO_LARGE;
    formatter::format_result(r7, out);
    CHECK_EQ(out, "Error: grid too large\n");

    // 8. registration already parked
    out.clear();
    CommandResult r8; r8.kind = ResultKind::ERROR_REGISTRATION_ALREADY_PARKED; r8.registration = "KA-01-1234";
    formatter::format_result(r8, out);
    CHECK_EQ(out, "Error: registration KA-01-1234 is already parked\n");

    // 9. slot out of range
    out.clear();
    CommandResult r9; r9.kind = ResultKind::ERROR_SLOT_OUT_OF_RANGE; r9.slot = {10, 20};
    formatter::format_result(r9, out);
    CHECK_EQ(out, "Error: slot (10, 20) is out of range\n");

    // 10. not usable slot
    out.clear();
    CommandResult r10; r10.kind = ResultKind::ERROR_NOT_USABLE_SLOT; r10.slot = {1, 1};
    formatter::format_result(r10, out);
    CHECK_EQ(out, "Error: (1, 1) is not a usable parking slot\n");

    // 11. slot already free
    out.clear();
    CommandResult r11; r11.kind = ResultKind::ERROR_SLOT_ALREADY_FREE; r11.slot = {2, 1};
    formatter::format_result(r11, out);
    CHECK_EQ(out, "Error: slot (2, 1) is already free\n");
}

int main() {
    return test_framework::run_all_tests();
}

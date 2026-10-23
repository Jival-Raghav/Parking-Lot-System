#include "test_framework.h"
#include "../src/commands.h"

using namespace parking;

static Command make_sample_create_cmd() {
    Command cmd;
    cmd.type = CommandType::CREATE_PARKING_LOT;
    cmd.raw_name = "create_parking_lot";
    cmd.rows = 5;
    cmd.cols = 7;
    cmd.map_is_valid = true;
    cmd.map_lines = {
        "E.....P",
        "P#P#P.P",
        "P.....#",
        "PP#P#.P",
        "P####.P"
    };
    return cmd;
}

TEST_CASE(test_commands_before_create) {
    LotState lot;
    Command park_cmd{CommandType::PARK, "park", {"KA-01-1234", "White"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r1 = commands::execute(lot, park_cmd);
    CHECK(r1.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r1.command_name, "park");
    CHECK(r1.reason == ErrorReason::PARKING_LOT_NOT_CREATED);

    Command leave_cmd{CommandType::LEAVE, "leave", {"2", "1"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r2 = commands::execute(lot, leave_cmd);
    CHECK(r2.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r2.command_name, "leave");
    CHECK(r2.reason == ErrorReason::PARKING_LOT_NOT_CREATED);

    Command status_cmd{CommandType::STATUS, "status", {}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r3 = commands::execute(lot, status_cmd);
    CHECK(r3.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r3.command_name, "status");
    CHECK(r3.reason == ErrorReason::PARKING_LOT_NOT_CREATED);

    Command reg_cmd{CommandType::REG_NUMS_FOR_COLOUR, "registration_numbers_for_cars_with_colour", {"White"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r4 = commands::execute(lot, reg_cmd);
    CHECK(r4.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r4.command_name, "registration_numbers_for_cars_with_colour");
    CHECK(r4.reason == ErrorReason::PARKING_LOT_NOT_CREATED);
}

TEST_CASE(test_create_and_already_created) {
    LotState lot;
    Command create_cmd = make_sample_create_cmd();
    CommandResult r1 = commands::execute(lot, create_cmd);
    CHECK(r1.kind == ResultKind::CREATE_SUCCESS);
    CHECK_EQ(r1.usable_slots_count, 10);

    // Second create -> LOT_ALREADY_EXISTS
    CommandResult r2 = commands::execute(lot, create_cmd);
    CHECK(r2.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r2.command_name, "create_parking_lot");
    CHECK(r2.reason == ErrorReason::LOT_ALREADY_EXISTS);
}

TEST_CASE(test_park_and_leave_errors) {
    LotState lot;
    commands::execute(lot, make_sample_create_cmd());

    // Park a car
    Command park1{CommandType::PARK, "park", {"KA-01-1111", "White"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r1 = commands::execute(lot, park1);
    CHECK(r1.kind == ResultKind::ALLOCATED_SLOT);
    CHECK_EQ(r1.slot.row, 2);
    CHECK_EQ(r1.slot.col, 1);

    // Duplicate registration
    Command park_dup{CommandType::PARK, "park", {"KA-01-1111", "Red"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_dup = commands::execute(lot, park_dup);
    CHECK(r_dup.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_dup.command_name, "park");
    CHECK(r_dup.reason == ErrorReason::REGISTRATION_ALREADY_PARKED);

    // Leave out of range
    Command leave_oor{CommandType::LEAVE, "leave", {"99", "99"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_oor = commands::execute(lot, leave_oor);
    CHECK(r_oor.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_oor.command_name, "leave");
    CHECK(r_oor.reason == ErrorReason::SLOT_OUT_OF_RANGE);

    // Leave non-usable slot (e.g. entrance at (1, 1))
    Command leave_not_slot{CommandType::LEAVE, "leave", {"1", "1"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_not_slot = commands::execute(lot, leave_not_slot);
    CHECK(r_not_slot.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_not_slot.command_name, "leave");
    CHECK(r_not_slot.reason == ErrorReason::NOT_A_USABLE_SLOT);

    // Leave unusable slot P at (4, 1)
    Command leave_unusable_p{CommandType::LEAVE, "leave", {"4", "1"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_unusable_p = commands::execute(lot, leave_unusable_p);
    CHECK(r_unusable_p.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_unusable_p.command_name, "leave");
    CHECK(r_unusable_p.reason == ErrorReason::NOT_A_USABLE_SLOT);

    // Leave already free usable slot (2, 3)
    Command leave_free{CommandType::LEAVE, "leave", {"2", "3"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_free = commands::execute(lot, leave_free);
    CHECK(r_free.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_free.command_name, "leave");
    CHECK(r_free.reason == ErrorReason::SLOT_ALREADY_FREE);

    // Valid leave
    Command leave_valid{CommandType::LEAVE, "leave", {"2", "1"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_valid = commands::execute(lot, leave_valid);
    CHECK(r_valid.kind == ResultKind::SLOT_IS_FREE);
    CHECK_EQ(r_valid.slot.row, 2);
    CHECK_EQ(r_valid.slot.col, 1);

    // Leaving again immediately -> SLOT_ALREADY_FREE
    CommandResult r_again = commands::execute(lot, leave_valid);
    CHECK(r_again.kind == ResultKind::COMMAND_FAILED);
    CHECK_EQ(r_again.command_name, "leave");
    CHECK(r_again.reason == ErrorReason::SLOT_ALREADY_FREE);
}

TEST_CASE(test_status_and_lookups) {
    LotState lot;
    commands::execute(lot, make_sample_create_cmd());

    // Status on empty lot
    Command status_cmd{CommandType::STATUS, "status", {}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_stat_empty = commands::execute(lot, status_cmd);
    CHECK(r_stat_empty.kind == ResultKind::EMPTY_LOT);

    // Park 2 cars
    commands::execute(lot, Command{CommandType::PARK, "park", {"CAR-1", "White"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS});
    commands::execute(lot, Command{CommandType::PARK, "park", {"CAR-2", "Red"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS});

    CommandResult r_stat = commands::execute(lot, status_cmd);
    CHECK(r_stat.kind == ResultKind::STATUS_TABLE);
    CHECK_EQ(r_stat.status_rows.size(), static_cast<size_t>(2));

    // Lookup reg by colour (case-insensitive)
    Command reg_lookup{CommandType::REG_NUMS_FOR_COLOUR, "registration_numbers_for_cars_with_colour", {"white"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_reg = commands::execute(lot, reg_lookup);
    CHECK(r_reg.kind == ResultKind::REGISTRATION_LIST);
    CHECK_EQ(r_reg.strings.size(), static_cast<size_t>(1));
    CHECK_EQ(r_reg.strings[0], "CAR-1");

    // Lookup slot by registration
    Command slot_lookup{CommandType::SLOT_FOR_REG, "slot_number_for_registration_number", {"CAR-1"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_slot = commands::execute(lot, slot_lookup);
    CHECK(r_slot.kind == ResultKind::SINGLE_SLOT);
    CHECK_EQ(r_slot.slot.row, 2);
    CHECK_EQ(r_slot.slot.col, 1);

    // Lookup non-existent reg
    Command slot_bad{CommandType::SLOT_FOR_REG, "slot_number_for_registration_number", {"CAR-UNKNOWN"}, 0, 0, {}, true, ErrorReason::INVALID_ARGUMENTS};
    CommandResult r_bad = commands::execute(lot, slot_bad);
    CHECK(r_bad.kind == ResultKind::NOT_FOUND);
}

int main() {
    return test_framework::run_all_tests();
}

#include "test_framework.h"
#include "../src/parser.h"
#include "../src/commands.h"
#include "../src/formatter.h"

using namespace parking;

TEST_CASE(test_pdf_sample_golden_byte_for_byte) {
    const std::string sample_input = 
        "create_parking_lot 5 7\n"
        "E.....P\n"
        "P#P#P.P\n"
        "P.....#\n"
        "PP#P#.P\n"
        "P####.P\n"
        "park KA-01-HH-1234 White\n"
        "park KA-01-HH-9999 White\n"
        "park KA-01-BB-0001 Black\n"
        "park KA-01-HH-7777 Red\n"
        "park KA-01-HH-2701 Blue\n"
        "park KA-01-HH-3141 Black\n"
        "park KA-01-HH-5555 Grey\n"
        "park KA-01-HH-6666 White\n"
        "park KA-01-HH-8888 Red\n"
        "park KA-01-HH-1111 Blue\n"
        "leave 1 7\n"
        "leave 2 1\n"
        "status\n"
        "park KA-01-P-333 White\n"
        "park DL-12-AA-9999 White\n"
        "park MH-04-AY-1111 Black\n"
        "registration_numbers_for_cars_with_colour White\n"
        "slot_numbers_for_cars_with_colour White\n"
        "slot_number_for_registration_number KA-01-HH-3141\n"
        "slot_number_for_registration_number MH-04-AY-1111\n"
        "slot_number_for_registration_number TN-07-ZZ-0000\n";

    const std::string expected_output = 
        "Created a parking lot with 10 usable slots\n"
        "Allocated slot: (2, 1)\n"
        "Allocated slot: (2, 3)\n"
        "Allocated slot: (2, 5)\n"
        "Allocated slot: (1, 7)\n"
        "Allocated slot: (2, 7)\n"
        "Allocated slot: (4, 7)\n"
        "Allocated slot: (4, 4)\n"
        "Allocated slot: (5, 7)\n"
        "Allocated slot: (3, 1)\n"
        "Allocated slot: (4, 2)\n"
        "Slot (1, 7) is free\n"
        "Slot (2, 1) is free\n"
        "Slot     Registration No    Colour\n"
        "(2, 3)   KA-01-HH-9999      White\n"
        "(2, 5)   KA-01-BB-0001      Black\n"
        "(2, 7)   KA-01-HH-2701      Blue\n"
        "(3, 1)   KA-01-HH-8888      Red\n"
        "(4, 2)   KA-01-HH-1111      Blue\n"
        "(4, 4)   KA-01-HH-5555      Grey\n"
        "(4, 7)   KA-01-HH-3141      Black\n"
        "(5, 7)   KA-01-HH-6666      White\n"
        "Allocated slot: (2, 1)\n"
        "Allocated slot: (1, 7)\n"
        "Sorry, parking lot is full\n"
        "DL-12-AA-9999, KA-01-P-333, KA-01-HH-9999, KA-01-HH-6666\n"
        "(1, 7), (2, 1), (2, 3), (5, 7)\n"
        "(4, 7)\n"
        "Not found\n"
        "Not found\n";

    auto cmds = parser::parse_input(sample_input);
    LotState lot;
    std::string actual_output;

    for (const auto& cmd : cmds) {
        CommandResult res = commands::execute(lot, cmd);
        formatter::format_result(res, actual_output);
    }

    CHECK_EQ(actual_output.size(), expected_output.size());
    CHECK_EQ(actual_output, expected_output);
}

int main() {
    return test_framework::run_all_tests();
}

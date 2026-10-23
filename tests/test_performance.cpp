#include "test_framework.h"
#include "../src/commands.h"
#include "../src/distance.h"
#include "../src/slots.h"
#include "../src/grid.h"
#include "../src/parser.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using namespace parking;
using namespace std::chrono;

TEST_CASE(test_perf_scenario_a_1000x1000_open_grid) {
    int32_t rows = 1000;
    int32_t cols = 1000;
    std::cout << "\n[PERF A] Generating 1000x1000 grid (500,000 P cells)..." << std::endl;

    Command cmd;
    cmd.type = CommandType::CREATE_PARKING_LOT;
    cmd.raw_name = "create_parking_lot";
    cmd.rows = rows;
    cmd.cols = cols;
    cmd.map_lines.reserve(static_cast<size_t>(rows));

    // First row: E then road cells
    std::string row0(static_cast<size_t>(cols), '.');
    row0[0] = 'E';
    cmd.map_lines.push_back(row0);

    // Remaining rows: alternating road and P columns (500 P cells per row)
    std::string regular_row(static_cast<size_t>(cols), '.');
    for (int c = 1; c < cols; c += 2) {
        regular_row[static_cast<size_t>(c)] = 'P';
    }
    for (int r = 1; r < rows; ++r) {
        cmd.map_lines.push_back(regular_row);
    }

    LotState lot;
    auto t0 = high_resolution_clock::now();
    CommandResult res = commands::execute_create_parking_lot(lot, cmd);
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();

    std::cout << "[PERF A] 1000x1000 lot created in " << ms << " ms. Usable slots: "
              << res.usable_slots_count << std::endl;

    CHECK(res.kind == ResultKind::CREATE_SUCCESS);
    CHECK(res.usable_slots_count >= 400000);
}

TEST_CASE(test_perf_scenario_b_100000_commands) {
    // 500x500 lot has 250,000 cells
    int32_t rows = 200;
    int32_t cols = 500;
    Command create_cmd;
    create_cmd.type = CommandType::CREATE_PARKING_LOT;
    create_cmd.raw_name = "create_parking_lot";
    create_cmd.rows = rows;
    create_cmd.cols = cols;

    std::string row0(static_cast<size_t>(cols), '.');
    row0[0] = 'E';
    create_cmd.map_lines.push_back(row0);

    std::string reg_row(static_cast<size_t>(cols), '.');
    for (int c = 1; c < cols; c += 2) reg_row[static_cast<size_t>(c)] = 'P';
    for (int r = 1; r < rows; ++r) create_cmd.map_lines.push_back(reg_row);

    LotState lot;
    commands::execute_create_parking_lot(lot, create_cmd);

    std::cout << "[PERF B] Running 100,000 mixed park/leave/lookup commands..." << std::endl;

    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < 40000; ++i) {
        std::string reg = "REG-" + std::to_string(i);
        std::string colour = (i % 2 == 0) ? "White" : "Black";
        Command park_cmd;
        park_cmd.type = CommandType::PARK;
        park_cmd.raw_name = "park";
        park_cmd.args = {reg, colour};
        commands::execute_park(lot, park_cmd);
    }

    for (int i = 0; i < 20000; ++i) {
        std::string reg = "REG-" + std::to_string(i * 2);
        Command lookup_cmd;
        lookup_cmd.type = CommandType::SLOT_FOR_REG;
        lookup_cmd.raw_name = "slot_number_for_registration_number";
        lookup_cmd.args = {reg};
        commands::execute_slot_for_reg(lot, lookup_cmd);
    }

    for (int i = 0; i < 20000; ++i) {
        std::string reg = "REG-" + std::to_string(i);
        auto it = lot.reg_to_cell.find(reg);
        if (it != lot.reg_to_cell.end()) {
            Position pos = grid::to_position(static_cast<size_t>(it->second), lot.cols);
            Command leave_cmd;
            leave_cmd.type = CommandType::LEAVE;
            leave_cmd.raw_name = "leave";
            leave_cmd.args = {std::to_string(pos.row), std::to_string(pos.col)};
            commands::execute_leave(lot, leave_cmd);
        }
    }

    for (int i = 0; i < 20000; ++i) {
        std::string reg = "NEW-REG-" + std::to_string(i);
        Command park_cmd;
        park_cmd.type = CommandType::PARK;
        park_cmd.raw_name = "park";
        park_cmd.args = {reg, "Blue"};
        commands::execute_park(lot, park_cmd);
    }
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();

    std::cout << "[PERF B] 100,000 commands executed in " << ms << " ms." << std::endl;
    CHECK(ms < 5000.0); // well within a few seconds
}

TEST_CASE(test_perf_scenario_c_adversarial_colour_queries) {
    std::cout << "[PERF C] Adversarial colour queries..." << std::endl;
    int32_t rows = 200;
    int32_t cols = 200;
    Command create_cmd;
    create_cmd.type = CommandType::CREATE_PARKING_LOT;
    create_cmd.rows = rows;
    create_cmd.cols = cols;
    std::string row0(static_cast<size_t>(cols), '.'); row0[0] = 'E';
    create_cmd.map_lines.push_back(row0);
    std::string reg_row(static_cast<size_t>(cols), '.');
    for (int c = 1; c < cols; c += 2) reg_row[static_cast<size_t>(c)] = 'P';
    for (int r = 1; r < rows; ++r) create_cmd.map_lines.push_back(reg_row);

    LotState lot;
    commands::execute_create_parking_lot(lot, create_cmd);

    // Park 15,000 cars of colour "White"
    for (int i = 0; i < 15000; ++i) {
        Command park_cmd;
        park_cmd.type = CommandType::PARK;
        park_cmd.raw_name = "park";
        park_cmd.args = {"CAR-" + std::to_string(i), "White"};
        commands::execute_park(lot, park_cmd);
    }

    auto t0 = high_resolution_clock::now();
    // 500 repeated colour lookups
    Command query_cmd;
    query_cmd.type = CommandType::REG_NUMS_FOR_COLOUR;
    query_cmd.raw_name = "registration_numbers_for_cars_with_colour";
    query_cmd.args = {"white"};
    for (int q = 0; q < 500; ++q) {
        CommandResult r = commands::execute_reg_nums_for_colour(lot, query_cmd);
        CHECK_EQ(r.strings.size(), static_cast<size_t>(15000));
    }
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();
    std::cout << "[PERF C] 500 queries over 15,000 same-colour cars in " << ms << " ms." << std::endl;
}

TEST_CASE(test_perf_scenario_d_snake_maze_deepest_bfs) {
    int rows = 1000;
    int cols = 1000;
    std::cout << "[PERF D] 1000x1000 snake maze (deepest iterative BFS, path length ~500,000)..." << std::endl;

    Command cmd;
    cmd.type = CommandType::CREATE_PARKING_LOT;
    cmd.rows = rows;
    cmd.cols = cols;
    cmd.map_lines.reserve(static_cast<size_t>(rows));

    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            cmd.map_lines.push_back(std::string(static_cast<size_t>(cols), '.'));
        } else {
            std::string wall_row(static_cast<size_t>(cols), '#');
            if ((r / 2) % 2 == 0) {
                wall_row[static_cast<size_t>(cols - 1)] = '.';
            } else {
                wall_row[0] = '.';
            }
            cmd.map_lines.push_back(wall_row);
        }
    }
    cmd.map_lines[0][0] = 'E';
    cmd.map_lines[static_cast<size_t>(rows - 1)][static_cast<size_t>(cols - 1)] = 'P';

    LotState lot;
    auto t0 = high_resolution_clock::now();
    CommandResult res = commands::execute_create_parking_lot(lot, cmd);
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();

    std::cout << "[PERF D] 1000x1000 snake maze solved in " << ms << " ms. Distance: "
              << lot.usable_slots[0].distance << std::endl;

    CHECK(res.kind == ResultKind::CREATE_SUCCESS);
    CHECK_EQ(res.usable_slots_count, 1);
    CHECK(lot.usable_slots[0].distance > 400000);
}

int main() {
    return test_framework::run_all_tests();
}

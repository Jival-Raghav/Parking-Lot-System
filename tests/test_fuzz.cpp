#include "test_framework.h"
#include "../src/parser.h"
#include "../src/commands.h"
#include <random>
#include <string>
#include <vector>
#include <set>
#include <iostream>

using namespace parking;

static const std::string BASE_SAMPLE = 
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

static void verify_invariants(const LotState& lot) {
    if (!lot.created) return;

    // 1. Every parked registration is in lot.reg_to_cell exactly once
    CHECK_EQ(lot.all_occupied.size(), lot.reg_to_cell.size());

    // 2. Occupied slots match usable ranks
    for (int32_t cell_idx : lot.all_occupied) {
        CHECK(cell_idx >= 0 && static_cast<size_t>(cell_idx) < lot.total_cells);
        int32_t rank = lot.cell_to_rank[static_cast<size_t>(cell_idx)];
        CHECK(rank >= 0 && rank < lot.usable_count);
        CHECK(lot.rank_occupant[static_cast<size_t>(rank)].has_value());
    }

    // 3. Invariant: a rank is in the free heap iff its slot is free
    // Extract ranks from heap
    auto heap_copy = lot.free_ranks;
    std::multiset<int32_t> free_ranks_multiset;
    while (!heap_copy.empty()) {
        free_ranks_multiset.insert(heap_copy.top());
        heap_copy.pop();
    }

    // No duplicate ranks in free heap!
    for (int32_t r = 0; r < lot.usable_count; ++r) {
        size_t count_in_heap = free_ranks_multiset.count(r);
        bool is_occupied = lot.rank_occupant[static_cast<size_t>(r)].has_value();

        if (is_occupied) {
            CHECK_EQ(count_in_heap, static_cast<size_t>(0));
        } else {
            CHECK_EQ(count_in_heap, static_cast<size_t>(1));
        }
    }
}

TEST_CASE(test_fuzz_thousands_mutations) {
    std::mt19937 rng(1337);
    const int NUM_ITERATIONS = 5000;

    std::cout << "\n[FUZZ] Running " << NUM_ITERATIONS << " fuzz iterations with invariant checks..." << std::endl;

    for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
        std::string mutated = BASE_SAMPLE;

        int mutation_type = rng() % 5;
        if (mutation_type == 0) {
            // Random byte replacement
            int count = 1 + (rng() % 20);
            for (int k = 0; k < count; ++k) {
                size_t pos = rng() % mutated.size();
                mutated[pos] = static_cast<char>(rng() % 256);
            }
        } else if (mutation_type == 1) {
            // Truncation
            size_t new_len = rng() % mutated.size();
            mutated.resize(new_len);
        } else if (mutation_type == 2) {
            // Insertion of random junk
            size_t pos = rng() % mutated.size();
            std::string junk;
            int jlen = 1 + (rng() % 50);
            for (int j = 0; j < jlen; ++j) junk.push_back(static_cast<char>(rng() % 256));
            mutated.insert(pos, junk);
        } else if (mutation_type == 3) {
            // Line shuffle
            auto lines = parser::split_lines(mutated);
            std::shuffle(lines.begin(), lines.end(), rng);
            mutated.clear();
            for (const auto& l : lines) {
                mutated += std::string(l) + "\n";
            }
        } else {
            // Deletion of slice
            if (mutated.size() > 10) {
                size_t pos = rng() % (mutated.size() - 5);
                size_t len = 1 + (rng() % 10);
                mutated.erase(pos, len);
            }
        }

        // Run in-process
        auto cmds = parser::parse_input(mutated);
        LotState lot;
        for (const auto& cmd : cmds) {
            commands::execute(lot, cmd);
            verify_invariants(lot);
        }
    }

    std::cout << "[FUZZ] Completed " << NUM_ITERATIONS << " fuzz iterations successfully!" << std::endl;
}

int main() {
    return test_framework::run_all_tests();
}

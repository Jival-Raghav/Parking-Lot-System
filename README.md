# Parking Lot System

A high-performance, modular, zero-dependency command-line parking lot management system built in C++17 according to the problem specification.

---

## 1. Build, Run, and Test Instructions

### Building with Make (GCC / Clang)

Compile `./parking` with default compiler (`g++`):
```bash
make
```

To compile with `clang++`:
```bash
make CXX=clang++
```

Both `g++` and `clang++` compile with flags: `-std=c++17 -O2 -Wall -Wextra -pedantic` with zero warnings.

### Running the Program

```bash
./parking <input_file>
```
Example:
```bash
./parking input.txt
```

### Running Tests

Run the full automated test suite:
```bash
make test
```
To run tests with `clang++`:
```bash
make test CXX=clang++
```

### Sanitizer Build (Address & Undefined Behavior Sanitizers)

Run the test suite under AddressSanitizer and UndefinedBehaviorSanitizer:
```bash
make sanitize
```

### Building with CMake (Windows MSVC / Linux / macOS)

Configure and build with CMake:
```bash
cmake -B build
cmake --build build --config Release
```
Run tests via CTest:
```bash
ctest --test-dir build -C Release --output-on-failure
```

---

## 2. Architecture & Module Overview

The architecture follows a strict modular, function-based, loosely-coupled design. No inheritance hierarchies, no singletons, no globals, and no hidden static state.
Core logic performs zero I/O (no `std::cout`, no `std::cin`, no file operations). All file I/O and stream output is strictly isolated to `src/main.cpp`.

```
src/
  types.h          # Data structures: Command, Position, UsableSlot, Car, LotState, CommandResult
  parser.h/.cpp    # Robust input parser: BOM stripping, line normalization, command tokenization
  grid.h/.cpp      # 2D flat grid coordinate transforms, 4-way neighbours, and map validation
  distance.h/.cpp  # Iterative BFS from entrance 'E' over road cells and slot distance calculation
  slots.h/.cpp     # Usable slot filtering, sorting by (distance, row, col), and rank mapping
  state.h/.cpp     # Free slots min-heap, occupancy records, registration & colour indexing
  commands.h/.cpp  # Pure command execution handlers returning CommandResult
  formatter.h/.cpp # The sole module responsible for rendering output strings
  main.cpp         # Entry point: reads input file, dispatches commands, single buffered write
tests/
  test_framework.h # Minimal header-only test harness (CHECK / CHECK_EQ macros, registry runner)
  test_grid.cpp    # Unit tests for grid coordinate conversion, bounds, and map validator
  test_distance.cpp# Unit tests for BFS road and slot distance calculation
  test_slots.cpp   # Unit tests for usable slot sorting and rank assignment
  test_state.cpp   # Unit tests for heap operations, car insertion, and index management
  test_commands.cpp# Unit tests for pure command execution and error states
  test_parser.cpp  # Unit tests for UTF-8 BOM, CRLF, tokenization, dimension validation
  test_formatter.cpp# Unit tests for exact output formatting and error strings
  test_sample.cpp  # Golden test: byte-for-byte verification against PDF sample output
  test_edge_cases.cpp# Complete suite covering all 60 edge cases from the specification
  test_performance.cpp# Performance benchmarks for 1000x1000 grids, 100k commands, and deep mazes
```

---

## 3. Algorithm & Computational Complexity

### Algorithm Details
1. **Precomputed Distances**: Walls and entrance never change and cars cannot cross slots. When a lot is created, **one iterative BFS** (using a flat `std::vector<int32_t>` queue with a head index) explores reachable road cells (`.` and `E`) starting from `E`.
2. **Slot Usability**: For every slot cell `'P'`, its distance is `min(neighbour road distance) + 1`. Slots with no reachable road neighbour (e.g., surrounded by walls or other `'P'` cells) have distance `-1` and are completely ignored.
3. **Rank Mapping**: All $S$ usable slots are sorted once by `(distance ASC, row ASC, col ASC)` and assigned integer ranks $0 \dots S-1$. We maintain `rank_to_cell` (vector) and `cell_to_rank` (flat vector of size $N = \text{rows} \times \text{cols}$, initialized to $-1$).
4. **Free Slot Management**: A min-heap (`std::priority_queue<int32_t, vector<int32_t>, greater<int32_t>>`) holds available ranks. Parking pops the smallest rank (closest slot). Leaving pushes the rank back to the heap.
5. **Ordered Indexes**:
   - `reg_to_cell`: `std::unordered_map<std::string, int32_t>` for $O(1)$ registration lookup.
   - `colour_to_cells`: `std::unordered_map<std::string, std::set<int32_t>>` with lowercased colour keys. Since flat cell index `r * cols + c` naturally encodes row-major order, iterating `std::set<int32_t>` yields results sorted by row, then column without extra sorting.
   - `all_occupied`: `std::set<int32_t>` tracks all occupied slot cells in row-major order for `status`.

### Complexity Table

| Operation | Time Complexity | Space Complexity | Notes |
| :--- | :--- | :--- | :--- |
| `create_parking_lot` | $O(N + S \log S)$ | $O(N)$ | One iterative BFS over $N$ grid cells + sorting $S$ usable slots. |
| `park` | $O(\log S)$ | $O(1)$ | Pop min from heap ($O(\log S)$) + map/set inserts ($O(\log S)$). |
| `leave` | $O(\log S)$ | $O(1)$ | $O(1)$ rank lookup via `cell_to_rank` + erase from set/map + push to heap. |
| `status` | $O(A)$ | $O(A)$ | $A$ = occupied cars. Iterates `all_occupied` set; output-bound. |
| `slot_number_for_registration_number` | $O(1)$ average | $O(1)$ | Hash table lookup in `reg_to_cell`. |
| `registration_numbers_for_cars_with_colour` | $O(K)$ | $O(K)$ | $K$ = cars of given colour. Output-bound iteration over pre-ordered set. |
| `slot_numbers_for_cars_with_colour` | $O(K)$ | $O(K)$ | $K$ = cars of given colour. Output-bound iteration over pre-ordered set. |

---

## 4. System Invariants

1. **Slot Usability Invariant**: Every occupied slot is a valid, usable slot (`cell_to_rank[cell] != -1`). Unusable slots are never allocated or freed.
2. **Registration Uniqueness Invariant**: Every parked registration appears exactly once in `reg_to_cell`. A duplicate registration is rejected before any state is mutated.
3. **Free Heap Invariant**: A rank $r \in [0, S-1]$ is in `free_ranks` if and only if slot $r$ is currently unoccupied (`!rank_occupant[r].has_value()`). A rejected leave never pushes a duplicate rank.
4. **Index Consistency Invariant**: For every parked car, its cell is present in `all_occupied`, `reg_to_cell`, and `colour_to_cells[to_lower(colour)]`. Upon `leave`, all three indexes are updated simultaneously, and empty colour sets are erased.

---

## 5. Error Messages Reference

All non-fatal errors print one line to `stdout` in the command's execution order; execution continues:

- `Error: parking lot not created`
- `Error: parking lot already created`
- `Error: unknown command '<name>'`
- `Error: invalid arguments for <command>`
- `Error: invalid dimensions`
- `Error: invalid map`
- `Error: grid too large`
- `Error: registration <reg> is already parked`
- `Error: slot (r, c) is out of range`
- `Error: (r, c) is not a usable parking slot`
- `Error: slot (r, c) is already free`

Fatal command-line errors (missing input file, unreadable file, path is a directory) print a diagnostic message to `stderr` and exit with status code `1`.

---

## 6. Decisions on Ambiguous Cases

- **Item 9 (Extra command-line arguments)**: The program accepts the input file path as the first argument (`argv[1]`). Any surplus command-line arguments are safely ignored.
- **Item 24 (Second valid `create_parking_lot`)**: When a valid `create_parking_lot` command is encountered after a lot already exists, the parser consumes the subsequent `rows` map lines so they are not misread as commands, outputs `Error: parking lot already created`, and preserves the existing lot state without alteration.
- **Item 25 (Invalid dimensions on `create_parking_lot`)**: If dimensions cannot be parsed, the parser cannot infer how many map lines were intended. It does not guess; succeeding lines are treated as ordinary commands, each producing an unknown command error.
- **Item 37 (Registration case sensitivity)**: Car registrations are compared and stored with exact case sensitivity (`KA-01-HH-1234` != `ka-01-hh-1234`).
- **Item 56 (Registration lookup)**: Registration lookup is case-sensitive, matching the case-sensitive storage policy.

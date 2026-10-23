# Parking Lot System

A high-performance, modular, zero-dependency command-line parking lot management system built in C++17 adhering strictly to the specification.

---

## 1. Build, Run, and Test Instructions

### Building with Make (GCC / Clang)

Compile `./parking` with `g++`:
```bash
make
```

To compile with `clang++`:
```bash
make CXX=clang++
```

Compiled with flags: `-std=c++17 -O2 -Wall -Wextra -pedantic` with zero warnings.

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

Run bad inputs test suite (diffing expected outputs and exit codes):
```bash
bash tests/run_bad_inputs.sh
```

### Sanitizer Build (AddressSanitizer & UndefinedBehaviorSanitizer)

Compile and run all unit tests and the 5,000-iteration randomized fuzz test under AddressSanitizer and UndefinedBehaviorSanitizer:
```bash
make sanitize
```

### Building with CMake (Windows MSVC / Linux / macOS)

```bash
cmake -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

---

## 2. Architecture & Module Overview

The system follows a strict modular, function-based design with plain structs, explicit state passing, and zero global or static hidden state.
Core logic performs zero I/O. All file access, standard error reporting, and final standard output writes are confined strictly to `src/main.cpp`.

```
src/
  types.h          # Data structures: Command, Position, UsableSlot, Car, LotState, CommandResult
  parser.h / .cpp  # Input parser: line normalization (\r\n, \r, \n), tokenization, map consumption
  grid.h / .cpp    # Flat 2D grid index math, 4-way neighbours, and map validation
  distance.h / .cpp# Single iterative BFS from entrance 'E' over road cells; slot distance calculation
  slots.h / .cpp   # Usable slot filtering, sorting by (distance, row, col), and rank mapping
  state.h / .cpp   # Free-slots min-heap, occupancy records, registration & colour indexing
  commands.h / .cpp# Pure command execution handlers returning CommandResult (no I/O)
  formatter.h / .cpp# The ONLY module responsible for rendering output strings
  main.cpp         # Entry point: binary file read, BOM & encoding check, buffered stdout flush
tests/
  test_framework.h # Header-only test harness (CHECK / CHECK_EQ macros, test runner)
  test_grid.cpp    # Unit tests for coordinates, neighbours, and map validator
  test_distance.cpp# Unit tests for BFS road and slot distance calculation
  test_slots.cpp   # Unit tests for usable slot sorting and rank assignment
  test_state.cpp   # Unit tests for heap operations, car insertion, and index management
  test_commands.cpp# Unit tests for pure command execution and error states
  test_parser.cpp  # Unit tests for BOM, line breaks, token validation, early stops
  test_formatter.cpp# Unit tests for exact output formatting and error strings
  test_sample.cpp  # Golden test: byte-for-byte verification against PDF sample output
  test_edge_cases.cpp# Unit test suite covering all edge cases
  test_performance.cpp# Benchmarks for 1000x1000 grids, 100k commands, and deep mazes
  test_fuzz.cpp    # In-process fuzzer with invariant validation under sanitizers
  run_bad_inputs.sh# Shell script verifying all bad input cases against golden .out files
  bad_inputs/      # Test corpus of malformed and adversarial input files
tools/
  gen_large.py     # Python script to generate arbitrary large benchmark inputs
```

---

## 3. Algorithm & Computational Complexity

### Algorithm Details
1. **Precomputed Distances**: Walls and entrance never move, and cars cannot drive through parking slots. When a lot is created, **one iterative BFS** (using a preallocated flat queue with a head pointer) computes shortest road distances from entrance `'E'` traversing road cells (`.` and `E`) only.
2. **Slot Usability**: For every slot cell `'P'`, its distance is $\min(\text{neighbour road distance}) + 1$. Slots with no reachable road neighbour (e.g. enclosed by walls or other `'P'` cells) have distance `-1` and are completely ignored.
3. **Rank Mapping**: All $S$ usable slots are sorted once by `(distance ASC, row ASC, col ASC)` and assigned integer ranks $0 \dots S-1$. We maintain `rank_to_cell` (vector) and `cell_to_rank` (flat vector of size $N = \text{rows} \times \text{cols}$, initialized to $-1$). This enables $O(1)$ rank lookup during `leave` without scanning.
4. **Free Slot Min-Heap**: A priority queue (`std::priority_queue<int32_t, vector<int32_t>, greater<int32_t>>`) holds available ranks. Parking pops the smallest rank (closest slot). Leaving pushes the rank back only after validating the slot was occupied.
5. **Ordered Indexes**:
   - `reg_to_cell`: `std::unordered_map<std::string, int32_t>` for $O(1)$ average registration lookup.
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

## 4. Error Handling Policy

The program never exhibits unknown behavior. Every bad input produces a short, predictable error line to STDOUT, avoids partial state mutation, and execution proceeds cleanly with the subsequent command.

### Standard Error Messages Table

Failed commands emit exactly one line to STDOUT in the format:
```
Error: <command_name> failed: <reason>
```

| Failed Command | Exact Output Message | Trigger Condition |
| :--- | :--- | :--- |
| *any command* | `Error: <cmd> failed: parking lot not created` | Executed prior to a successful `create_parking_lot`. |
| `create_parking_lot` | `Error: create_parking_lot failed: invalid arguments` | Wrong argument count, non-digit characters, zero, or control characters. |
| `create_parking_lot` | `Error: create_parking_lot failed: grid too large` | Declared $\text{rows} \times \text{cols} > 20,000,000$. Checked before reading map lines or allocating memory. |
| `create_parking_lot` | `Error: create_parking_lot failed: invalid map` | Early EOF, row length mismatch, characters other than `E P . #`, zero or multiple `E`, or early stop. |
| `create_parking_lot` | `Error: create_parking_lot failed: lot already exists` | Attempting to create a parking lot when one has already been created. Existing lot remains untouched. |
| `park` | `Error: park failed: invalid arguments` | Argument count != 2, token length > 256 bytes, or control characters present. |
| `park` | `Error: park failed: registration already parked` | Car with identical registration number is already inside the lot. |
| `leave` | `Error: leave failed: invalid arguments` | Argument count != 2, non-digit characters (including signs, commas, parens), zero row/col, or control characters. |
| `leave` | `Error: leave failed: slot out of range` | Numeric row/column is outside the lot boundaries or exceeds the saturated cap. |
| `leave` | `Error: leave failed: not a usable slot` | Coordinate refers to a wall, road, entrance, or unreachable parking slot. |
| `leave` | `Error: leave failed: slot already free` | Coordinate is a usable parking slot, but currently has no car parked in it. |
| `status` | `Error: status failed: invalid arguments` | Any extra arguments passed to `status`. |
| *lookups* | `Error: <cmd> failed: invalid arguments` | Argument count != 1, token length > 256 bytes, or control characters present. |
| *unknown command* | `Error: unknown command '<name>'` | Command token not recognized. Name is sanitized: printable ASCII only, other bytes `'?'`, max 32 chars then `"..."`. |
| *stray map lines* | `Error: unexpected map data ignored` | Emitted once per contiguous run of unexpected map-like lines outside a map block. |

*(Note: When a parking lot is completely full, `park` prints `Sorry, parking lot is full`. This is standard domain feedback, not an error).*

### Stream Alignment & Recovery Rules

1. **Early Stop on Incomplete Map**:
   While consuming the `rows` lines of a map block, if the first token of a line matches a known command name (`create_parking_lot`, `park`, `leave`, `status`, `registration_numbers_for_cars_with_colour`, `slot_numbers_for_cars_with_colour`, `slot_number_for_registration_number`), map consumption ends immediately *without* consuming that line. The create command fails with `invalid map`, and the recognized command line is processed as the next normal command.
2. **Orphaned Map Swallowing**:
   When `create_parking_lot` fails (e.g. invalid arguments, grid too large, or invalid map), the declared block size is untrusted or the map is abandoned. Any immediate following lines consisting only of `E P . #` are treated as the orphaned map and are silently swallowed without error, avoiding spurious unknown-command cascades. Swallowing terminates as soon as a non-map-like line is encountered.
3. **Stray Map Data Reporting**:
   If a valid create is followed by additional map-like lines (i.e. declared rows was smaller than the map in the file), or if map-like lines appear between valid commands, exactly one `Error: unexpected map data ignored` line is printed for the entire contiguous run, and the run is skipped.

### Process Exit Codes

- **Exit 0**: The input file was successfully read and all commands processed to completion, even if individual commands failed with error lines.
- **Exit 1 (Message to STDERR)**: Fatal invocation issues:
  - Missing file argument (`Usage: ./parking <input_file>`)
  - File does not exist, cannot be opened, or path is a directory
  - Unsupported encoding (UTF-16/32 BOM, or >10% NUL bytes in first 4KB)
  - Write error to stdout
- **Exit 2 (Message to STDERR)**: Uncaught runtime exception (`Fatal: internal error`). Any buffered output is flushed first.

### Documented Assumptions & Limitations

- **Spaces in Registrations**: Tokens are split on spaces and tabs. Registration numbers containing spaces cannot be parsed and will trigger `invalid arguments`.
- **Case Sensitivity**:
  - Command names are strictly case-sensitive (`park` is valid; `PARK` yields unknown command).
  - Registration numbers are strictly case-sensitive (`KA-01-HH-1234` != `ka-01-hh-1234`).
  - Colour lookups are ASCII case-insensitive (`White` matches `white` and `WHITE`). Non-ASCII bytes in colours are compared as opaque bytes.
- **Token Length Cap**: Registration and colour tokens are capped at 256 bytes. Tokens exceeding 256 bytes are rejected with `invalid arguments`.
- **Grid Cell Cap**: Total grid cells ($\text{rows} \times \text{cols}$) is capped at $20,000,000$ to prevent out-of-memory denial of service. Saturated arithmetic prevents integer overflow.

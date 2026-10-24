# Parking Lot System

A high-performance, modular, zero-dependency command-line parking lot management system built in C++17 adhering strictly to the specification.

---

## 1. Build and Run

> **Output Destination**: Output is written directly to standard output (**`stdout`**). To save output to a text file, use shell redirection (`> output.txt`).

### On Linux / macOS / WSL

#### Prerequisites
- A C++17 compiler (`g++` 8+ or `clang++` 7+)
- `make` (GNU Make 4.0+)

#### Build Commands
Using `make` (defaults to `g++` with `-O2 -Wall -Wextra -pedantic`):
```bash
make
```
Or to compile using `clang++`:
```bash
make CXX=clang++
```
Or direct compilation without `make`:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/*.cpp -o parking
```

#### Run Command
```bash
# Prints output directly to terminal (stdout)
./parking input.txt

# Or redirect stdout to a text file
./parking input.txt > output.txt
```

---

### On Windows

#### Prerequisites
Choose one of the following toolchains:
1. **MSVC** (Visual Studio 2019+ or Build Tools via "Developer Command Prompt")
2. **MinGW-w64** (`g++`)
3. **CMake** (3.15+) with any generator

#### Option A: Using MSVC (Developer Command Prompt / PowerShell)
```cmd
cl /std:c++17 /O2 /W4 /EHsc src/*.cpp /Feparking.exe
```
Run:
```cmd
parking.exe input.txt
```

#### Option B: Using MinGW-w64 (g++)
```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/*.cpp -o parking.exe
```
Run:
```powershell
.\parking.exe input.txt
```

#### Option C: Using CMake (Cross-platform)
```powershell
cmake -B build
cmake --build build --config Release
```
Run:
```powershell
.\build\Release\parking.exe input.txt
```

#### Option D: Using Windows Subsystem for Linux (WSL)
If using WSL from Windows PowerShell:
```powershell
wsl make
wsl ./parking input.txt
```

---

## 2. Test Instructions

All tests utilize a zero-dependency, self-written header-only test harness (`tests/test_framework.h`). No third-party test libraries (GoogleTest / Catch2) are required.

### Optional: Python Environment (for Benchmark Tools & Bad Inputs Generator)
If you wish to run the auxiliary benchmark tools (`tools/gen_large.py`) or regenerate the bad inputs corpus:

- **On Linux / macOS / WSL**:
  ```bash
  python3 -m venv venv
  source venv/bin/activate
  ```
- **On Windows (PowerShell)**:
  ```powershell
  python -m venv venv
  .\venv\Scripts\Activate.ps1
  ```

---

### Running Tests on Linux / macOS / WSL

#### 1. Full Unit & Benchmark Test Suite
Compiles all 11 test suites and runs unit tests, golden sample tests, edge cases, and the 5,000-iteration randomized fuzzer:
```bash
make test
```

#### 2. Memory Safety & Sanitizer Suite (AddressSanitizer & UBSan)
Compiles all tests with `-fsanitize=address,undefined -g` to verify absence of memory leaks, buffer overruns, and undefined behavior:
```bash
make sanitize
```

#### 3. Bad Inputs & Error Handling Verification
Executes 45 adversarial test cases (malformed maps, encoding violations, oversized tokens, out-of-order commands) and validates exact output strings and exit codes:
```bash
bash tests/run_bad_inputs.sh
```

#### 4. Standalone Fuzz Test
Runs 5,000 randomized state mutations with strict bidirectional invariant checking:
```bash
./build/test_fuzz
```

---

### Running Tests on Windows

Using CMake and CTest:
```powershell
cmake -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

---

## 3. Performance Benchmarks & Metrics

The system was benchmarked on a **$1000 \times 1000$ grid ($1,000,000$ cells)** and **100,000 commands**, evaluated against the problem statement requirement (*"handle a 1000 × 1000 grid and 100,000 commands in a few seconds"*).

### Measured Performance Results

| Benchmark Scenario | Dataset / Workload Details | Measured Execution Time | Memory Footprint (RAM) | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Real Binary End-to-End** (`./parking benchmark_100k.txt > /dev/null`) | Full program run: disk file read, 1M grid parsing, BFS, 100k mixed commands (40k park, 20k lookup, 20k leave, 20k park), formatted output | **0.65 seconds** (650 ms) | **$\approx 89 \text{ MB}$** | **PASS** (target: few seconds) |
| **Scenario A: 1000×1000 Open Grid** | BFS traversal from entrance across 1M cells, identifying and sorting 499,500 usable slots | **171.4 ms** | $\approx 28 \text{ MB}$ | **PASS** |
| **Scenario B: 100,000 In-Memory Commands** | 40,000 parks + 20,000 lookups + 20,000 leaves + 20,000 parks | **88.2 ms** | $\approx 36 \text{ MB}$ | **PASS** |
| **Scenario C: Adversarial Colour Queries** | 500 repeated colour lookups over 15,000 cars of same colour (7,500,000 item lookups) | **235.7 ms** | $\approx 22 \text{ MB}$ | **PASS** |
| **Scenario D: 1000×1000 Snake Maze** | Deepest iterative BFS path (maze depth ~500,000 road steps) | **19.4 ms** | $\approx 16 \text{ MB}$ | **PASS** |
| **Scenario E: Randomized Fuzzer** | 5,000 randomized command mutations with strict state invariant checks | **11.2 ms** | $\approx 4 \text{ MB}$ | **PASS** |
| **Golden Sample Verification** (`./parking input.txt`) | Byte-for-byte exact diff comparison against problem statement specification | **< 2 ms** | $\approx 2 \text{ MB}$ | **PASS** (Zero diff) |

---

## 4. Architecture & Module Overview

The codebase enforces strict separation of concerns, loose coupling, and a **zero-I/O core policy**: no module inside `src/` (except `main.cpp`) touches `std::cout`, `std::cin`, files, or standard streams.

```
Latlong/
├── src/                  # Core Production Source Code (Zero I/O)
│   ├── types.h           # Data structures: Command, Position, UsableSlot, Car, LotState, CommandResult
│   ├── parser.h / .cpp   # Line normalization (\r\n, \r, \n), tokenization, saturating number parsing
│   ├── grid.h / .cpp     # Flat 2D grid index arithmetic, 4-way neighbours, coordinate validation
│   ├── distance.h / .cpp # Single iterative BFS from entrance 'E'; slot distance computation
│   ├── slots.h / .cpp    # Usable slot filtering, sorting by (distance, row, col), rank mapping
│   ├── state.h / .cpp    # Free-slots min-heap, occupancy state, registration and colour indexing
│   ├── commands.h / .cpp # Pure command execution handlers returning CommandResult
│   ├── formatter.h / .cpp# The ONLY module that builds output text and error strings
│   └── main.cpp          # Entry point: binary file I/O, BOM/encoding checks, 32MB buffered flush
│
├── tests/                # Automated Test Suites & Verification Corpus
│   ├── test_framework.h  # Tiny self-written header-only test harness (CHECK / CHECK_EQ macros)
│   ├── test_sample.cpp   # Golden test: byte-for-byte verification against PDF sample output
│   ├── test_commands.cpp # Unit tests for pure command execution and error states
│   ├── test_distance.cpp # Unit tests for BFS road and slot distance calculation
│   ├── test_edge_cases.cpp# 46 exhaustive unit tests covering edge cases & CR error handling
│   ├── test_formatter.cpp# Unit tests for output formatting, alignment, and command sanitization
│   ├── test_fuzz.cpp     # 5,000-iteration randomized fuzzer with strict state invariant checks
│   ├── test_grid.cpp     # Unit tests for coordinates, neighbours, and map validator
│   ├── test_parser.cpp   # Unit tests for BOM stripping, line breaks, token validation, early stops
│   ├── test_performance.cpp# Performance benchmarks for 1000x1000 grids and 100k commands
│   ├── test_slots.cpp    # Unit tests for usable slot sorting and rank assignment
│   ├── test_state.cpp    # Unit tests for heap operations, car insertion, and index management
│   ├── run_bad_inputs.sh # Shell test runner verifying bad input corpus against expected outputs
│   └── bad_inputs/       # Corpus of malformed/adversarial inputs (Categories A, B, C, D, E)
│
├── tools/                # Auxiliary Generation Scripts
│   ├── gen_large.py      # Benchmark generator for large grids & command sequences
│   └── create_bad_inputs.py# Generator script for tests/bad_inputs corpus
│
├── Makefile              # Build automation for Linux/macOS/WSL (make, make test, make sanitize)
├── CMakeLists.txt        # Cross-platform CMake build configuration
├── input.txt             # Original sample input from specification
├── expected.txt          # Golden reference output matching input.txt
└── README.md             # Project documentation
```

### Source Modules (`src/`)
- **`types.h`**: Common types (`Position`, `Car`, `UsableSlot`, `ErrorReason`, `CommandResult`). All lot state is bundled in explicit struct `LotState`.
- **`parser.h / .cpp`**: Splits lines on `\r\n`, lone `\r`, or lone `\n`. Enforces 256-byte token caps, rejects control characters, and saturates numbers without 64-bit integer overflow. Handles map block consumption and stops early upon encountering known commands.
- **`grid.h / .cpp`**: Translates between 1-based `(row, col)` coordinates and 0-based flat cell indices `(r - 1) * cols + (c - 1)`. Computes 4-way orthogonal neighbours.
- **`distance.h / .cpp`**: Executes **one iterative BFS** (no recursion) from `'E'` through road cells (`.` and `E`) using a preallocated flat queue. Determines slot distances as $\min(\text{road neighbour}) + 1$.
- **`slots.h / .cpp`**: Sorts usable slots by `(distance ASC, row ASC, col ASC)` and assigns ranks $0 \dots k-1$. Maintains `rank_to_cell` and flat `cell_to_rank` lookup vectors for $O(1)$ slot lookups.
- **`state.h / .cpp`**: Manages the `std::priority_queue` min-heap of free ranks, bidirectional car registration lookup (`reg_to_cell`), and row-major colour indexing (`colour_to_cells`).
- **`commands.h / .cpp`**: Dispatches commands against `LotState`. Validates all inputs locally before mutating any state (zero partial mutation on failure).
- **`formatter.h / .cpp`**: Sole location responsible for text generation. Formats status tables, comma-separated lists, and error messages. Sanitizes unknown command names (printable ASCII, max 32 chars).
- **`main.cpp`**: Sole location handling I/O. Reads files in binary mode, verifies UTF-8/UTF-16/UTF-32 encodings and NUL byte ratios, executes commands, buffers output, and flushes in 32 MB chunks.

### Test Suites (`tests/`)
- **`test_sample.cpp`**: Verifies that the program matches the problem statement PDF sample byte-for-byte.
- **`test_performance.cpp`**: Asserts that 1M-cell grids and 100k commands execute in under 1 second.
- **`test_fuzz.cpp`**: Executes thousands of random operations verifying that free slot counts, occupied slot counts, and indexes match after every single step.
- **`test_edge_cases.cpp`**: Tests all edge cases including BOM handling, CRLF/CR, long lines, full lots, duplicate registrations, and invalid coordinates.
- **`bad_inputs/` & `run_bad_inputs.sh`**: 45 adversarial test inputs testing every failure code, stream recovery, orphan map swallowing, and process exit codes.

---

## 5. Algorithm & Computational Complexity

### Algorithm Details
1. **Precomputed Distances**: Walls and entrance never move, and cars cannot drive through parking slots. When a lot is created, **one iterative BFS** computes shortest road distances from entrance `'E'` traversing road cells (`.` and `E`) only.
2. **Slot Usability**: For every slot cell `'P'`, its distance is $\min(\text{neighbour road distance}) + 1$. Slots with no reachable road neighbour have distance `-1` and are completely ignored.
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

## 6. Error Handling Policy

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

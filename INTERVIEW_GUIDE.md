# Parking Lot Management System — Master Interview Revision Manual

---

## 1. The 30-Second Elevator Pitch
> *"Tell me about this project and what you built."*

> "I built a high-performance, zero-dependency Parking Lot Management System in C++17. Unlike standard toy parking lots that allocate slots sequentially (1, 2, 3...), this system models a real-world 2D grid containing an entrance ('E'), roads ('.'), walls ('#'), and parking slots ('P').
>
> The core algorithmic challenge was dynamic nearest-slot allocation based on actual driving road distance from the entrance—not straight-line Euclidean distance—with deterministic tie-breaking by row and column.
>
> I engineered the system around a Zero-I/O Core Architecture where all domain logic is purely memory-efficient and isolated from stream I/O. The system exceeds benchmark requirements: it processes a 1,000,000-cell grid with 100,000 mixed commands in just 0.65 seconds while maintaining zero memory leaks under AddressSanitizer."

---

## 2. Complete End-to-End Execution Workflow

```
Disk File (binary)
       │
       ▼
1. main.cpp ─────────► [is_unsupported_encoding()] ──► Exit 1 on UTF-16
       │
       ▼ [Strip UTF-8 BOM EF BB BF]
2. parser.cpp ───────► [split_lines & tokenize] ────► [parse_input()] ──► std::vector<Command>
       │
       ▼
3. main.cpp ─────────► Top-Level Loop: for (cmd : cmds)
       │
       ▼
4. commands.cpp ─────► [Guard Clauses: state::is_usable_slot, is_slot_occupied, is_registration_parked]
       │                 (Delegates to: grid.cpp, distance.cpp, slots.cpp, state.cpp)
       │
       ▼ Returns CommandResult (Pure Data Struct)
5. formatter.cpp ────► [format_result()] ──► Appends to out_buffer (Aligned tables / Clean strings)
       │
       ▼
6. main.cpp ─────────► High-Throughput 32 MB Flush via binary std::fwrite() to stdout
```

---

## 3. Step-by-Step Command Lifecycle Traces

### A. Lifecycle of `create_parking_lot R C`
1. `commands.cpp` verifies `!lot.created`. If a lot already exists, returns `LOT_ALREADY_EXISTS`.
2. `grid::validate_map()` verifies:
   - Exactly $R$ rows exist and each row has $C$ columns.
   - Only valid characters `{'E', '.', '#', 'P'}` are present.
   - Exactly **ONE entrance `'E'`** is in the grid.
3. `commands.cpp` builds `LotState new_lot` **locally on the stack** (Atomic Transaction pattern).
4. `distance::compute_road_distances()` runs **ONE iterative BFS** from `'E'` through road cells (`.` and `E`). Returns flat array `road_dist`.
5. `slots::build_usable_slots()`:
   - For every `'P'` cell, calculates `slot_distance = min(road neighbours) + 1`.
   - Disconnected/walled-in slots with distance `-1` are completely filtered out.
   - Sorts usable slots once using `std::sort` by `(distance ASC, row ASC, col ASC)`.
   - Maps each slot to an integer rank $0 \dots S-1$ and fills the `free_ranks` min-heap.
6. `commands.cpp` commits: `lot = std::move(new_lot)`. Returns `usable_slots_count`.

---

### B. Lifecycle of `park <REG> <COLOUR>`
1. `commands.cpp` verifies lot is created and arguments are valid strings ($\le 256$ chars, no control chars).
2. **Guard Clause**: Calls `state::is_registration_parked(lot, reg)` in $O(1)$. If car already parked $\to$ returns `REGISTRATION_ALREADY_PARKED`.
3. **Capacity Check**: Checks `if (lot.free_ranks.empty())` $\to$ returns `LOT_FULL` (*"Sorry, parking lot is full"*).
4. **State Mutation (`state::park_car`)**:
   - Pops the lowest rank from `lot.free_ranks` min-heap in **$O(\log S)$**.
   - Converts rank to flat cell index via `lot.rank_to_cell[rank]`.
   - Inserts into `reg_to_cell[reg]` hash map in **$O(1)$** average.
   - Inserts cell index into `colour_to_cells[colour_lower]` set in **$O(\log K)$**.
   - Inserts cell index into `all_occupied` set in **$O(\log N_{\text{occ}})$**.
   - Returns `Position{row, col}` converted via `grid::to_position()`.
5. `formatter.cpp` formats: `"Allocated slot: (r, c)\n"`.

---

### C. Lifecycle of `leave <ROW> <COL>`
1. `commands.cpp` parses row and col, validating coordinates within bounds ($1 \le r \le R, 1 \le c \le C$).
2. Converts `(r, c)` to flat cell index using `grid::to_cell_index(r, c, cols)`.
3. **Guard Clause 1**: Calls `state::is_usable_slot(lot, cell_idx)`:
   - Checks `cell_to_rank[cell_idx] != -1`.
   - If false (it's an entrance, road, wall, or walled slot) $\to$ returns `NOT_A_USABLE_SLOT`.
4. **Guard Clause 2**: Calls `state::is_slot_occupied(lot, cell_idx)`:
   - Checks `rank_occupant[rank].has_value()`.
   - If false (no car parked) $\to$ returns `SLOT_ALREADY_FREE`.
5. **State Mutation (`state::free_slot`)**:
   - Erases car from `reg_to_cell`, `colour_to_cells`, and `all_occupied`.
   - Clears `rank_occupant[rank] = std::nullopt`.
   - **Recycles the slot**: Pushes the integer `rank` back into the `free_ranks` min-heap in **$O(\log S)$**!
6. `formatter.cpp` formats: `"Slot (r, c) is free\n"`.

---

### D. Lifecycle of `status`
1. Verifies lot is created and arguments are empty.
2. If `lot.all_occupied.empty()` $\to$ returns `EMPTY_LOT` (*"Parking lot is empty"*).
3. Iterates over `lot.all_occupied` (`std::set<int32_t>`).
   - Because cell index `r * cols + c` naturally sorts by row then column, **zero sorting is needed at query time!**
4. `formatter.cpp` formats the 3-column table with dynamic spacing:
   ```
   Slot     Registration No    Colour
   (2, 3)   KA-01-HH-9999      White
   ```

---

## 4. Master Catalog of Exception & Error Scenarios

| Input Scenario | How Detected & Handled | Output & Behavior |
| :--- | :--- | :--- |
| **Command before create** | `commands.cpp` checks `!lot.created` | `Error: <cmd> failed: parking lot not created` |
| **Invalid create dims (0, neg, decimals, words)** | Saturating parser rejects non-digits; orphan map skip activates | `Error: create_parking_lot failed: invalid arguments` (following map lines silently skipped) |
| **Dimensions > 20,000,000 cells** | Pre-allocation check prevents out-of-memory crash | `Error: create_parking_lot failed: grid too large` |
| **Bad map chars (p, 1, e, X)** | `grid::validate_map()` rejects chars outside `{E, ., #, P}` | `Error: create_parking_lot failed: invalid map` |
| **Row length mismatch (short / long)** | `grid::validate_map()` checks `line.size() == cols` | `Error: create_parking_lot failed: invalid map` |
| **Invalid entrance count (0 or 2+ E)** | `grid::validate_map()` checks `entrance_count == 1` | `Error: create_parking_lot failed: invalid map` |
| **Declared rows > map rows (early stop)** | Parser sees known command (e.g. `park`) $\to$ stops consuming map rows | `Error: create_parking_lot failed: invalid map`, `park` executed next |
| **Declared rows < map rows** | Creates lot with declared rows; emits warning for excess rows | `Created lot...` then `Error: unexpected map data ignored` |
| **Blank line between create and map** | Parser treats blank line as map termination | `Error: create_parking_lot failed: invalid map` |
| **Second create while lot exists** | Guard check `lot.created` triggers; old cars preserved | `Error: create_parking_lot failed: lot already exists` |
| **Walled-in / unreachable slots** | BFS road distance is `-1`; `compute_slot_distance` returns `-1` | Slot filtered out; if 0 slots: `Created a parking lot with 0 usable slots` |
| **Park when lot is full** | `free_ranks` min-heap is empty | `Sorry, parking lot is full` |
| **Duplicate car registration** | `state::is_registration_parked()` finds plate in `reg_to_cell` | `Error: park failed: registration already parked` |
| **Malformed leave (words, decimals)** | Saturating number parser rejects non-digits | `Error: leave failed: invalid arguments` |
| **Leave coordinate outside grid** | Checks $r > \text{rows}$ or $c > \text{cols}$ | `Error: leave failed: slot out of range` |
| **Leave on road, wall, entrance** | `state::is_usable_slot()` returns false (`cell_to_rank == -1`) | `Error: leave failed: not a usable slot` |
| **Leave on already-empty slot** | `state::is_slot_occupied()` returns false (no occupant) | `Error: leave failed: slot already free` |
| **Unknown command (PARK, hello)** | Parser flags unknown token; name sanitized to printable ASCII | `Error: unknown command '<sanitized>'` |
| **Missing / extra query args** | Argument count checked in `commands.cpp` | `Error: <cmd> failed: invalid arguments` |
| **Plate or colour not found** | Hash lookup returns empty set or end iterator | `Not found` |
| **UTF-16 / UTF-32 encoded file** | `main.cpp` checks binary BOM (`FF FE`, `FE FF`); exits immediately | Nothing on stdout; stderr: `Error: unsupported file encoding` (Exit code 1) |

---

## 5. Summary of Key Architectural Choices

1. **Why `const LotState& lot` in checks?**
   Passing by const-reference means zero memory copying (0 nanoseconds), zero heap allocation, and a compile-time guarantee that checking functions cannot mutate state.
2. **Why `inline` in `grid.h`?**
   Allows the compiler to substitute coordinate conversion math `(r-1)*cols + (c-1)` directly into calling code, completely eliminating CPU function call overhead.
3. **Why Flat 1D Array instead of `vector<vector<T>>`?**
   Eliminates pointer chasing and memory fragmentation. Keeps all 1,000,000 cells in contiguous memory for maximum L1/L2 cache locality.
4. **Why `std::sort` on vector instead of `std::set` in `slots.cpp`?**
   `std::set` allocates tree nodes on the heap for every element. `std::sort` on a contiguous `std::vector` is 10x faster due to sequential cache line prefetching.
5. **Why Monotonic Rank Mapping with Min-Heap?**
   Slots are sorted once upfront. The min-heap stores simple 32-bit integer ranks (`int32_t`). Popping and pushing ranks runs in $O(\log S)$ with minimal CPU cycles.
6. **What is CMake vs. Make?**
   Make is a build tool that executes a `Makefile` directly. CMake is a meta-build generator that creates native Makefiles or Visual Studio projects from `CMakeLists.txt`.

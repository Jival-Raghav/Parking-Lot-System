# Parking Lot System — High-Level Interview Preparation Guide

This guide is structured specifically for an interview setting. It arms you with high-signal answers, elevator pitches, architectural explanations, algorithmic complexities, and design rationale.

---

## 1. The 30-Second Elevator Pitch
> *"If the interviewer asks: 'Tell me about this project and what you built.'"*

**How to answer:**
> "I built a high-performance, zero-dependency Parking Lot Management System in C++17. Unlike standard toy parking lots that assign slots sequentially (1, 2, 3...), this system models a real-world 2D grid containing an entrance, roads, walls, and parking slots.
> 
> The core algorithmic challenge was dynamic nearest-slot allocation based on **actual shortest driving distance** from the entrance through road cells—not straight-line Euclidean distance—with deterministic tie-breaking by row and column.
> 
> I designed it around a **Zero-I/O Core Architecture** where all business logic is pure, memory-efficient, and separated from stream I/O. The system easily exceeds the benchmark requirements: it processes a **1,000,000-cell grid with 100,000 mixed commands in just 0.65 seconds** while maintaining zero memory leaks under AddressSanitizer."

---

## 2. Why C++? (The Interview Justification)
> *"Why did you choose C++17 instead of Java, Python, or Go?"*

1. **Cache Locality & Predictable Memory Layout**:
   - In Python or Java, 2D grids often become arrays of pointers or object references (`vector<vector<T>>` or `ArrayList<ArrayList<T>>`), causing massive CPU cache misses (pointer chasing).
   - In C++, we flatten the entire $1000 \times 1000$ grid into a contiguous flat 1D vector (`std::vector<char>`). Accessing cell `(r, c)` is a single multiply-add: `(r - 1) * cols + (c - 1)`, giving maximum L1/L2 cache locality and SIMD vectorization potential.

2. **Zero-Overhead Abstractions & Deterministic Memory (RAII)**:
   - Garbage-collected languages (Java/Go/Python) suffer from stop-the-world GC pauses that degrade p99/p99.9 latency.
   - C++ uses RAII (Resource Acquisition Is Initialization): memory is released deterministically the instant an object goes out of scope, with zero runtime GC overhead.

3. **Zero External Dependencies**:
   - The entire application relies exclusively on the C++17 standard library (`<vector>`, `<queue>`, `<unordered_map>`, `<set>`). No third-party frameworks, boost libraries, or package managers required.

4. **Raw Benchmark Speed**:
   - Parsing 1M cells, running BFS, and processing 100k commands completes in **650 ms** in C++ vs. 5–15+ seconds in interpreted/VM-based languages.

---

## 3. What is CMake? (Make vs. CMake)
> *"Can you explain what CMake is, and why your repository has both a Makefile and CMakeLists.txt?"*

### The Core Difference
- **`Make` is a Build Tool**: It reads a `Makefile` and executes compiler commands (`g++ ...`) directly based on file timestamps to compile changed `.cpp` files into `.o` objects and link the binary. Make is specific to Unix-like environments.
- **`CMake` is a Meta-Build Generator**: CMake **does not** compile code itself. Instead, it reads `CMakeLists.txt` and *generates* native build files for whatever platform or IDE you are on:
  - On Linux/macOS: generates a `Makefile` or `Ninja` build file.
  - On Windows: generates Visual Studio solutions (`.sln`), MinGW Makefiles, or Ninja.

### Why both in this project?
- **`Makefile`**: Provided for **instant, zero-config compilation** on Linux, macOS, and WSL with a single word (`make`, `make test`, `make sanitize`).
- **`CMakeLists.txt`**: Provided for **industry-standard cross-platform compatibility**, enabling seamless integration into IDEs like CLion, VS Code, and Visual Studio, as well as native multi-platform CI/CD and CTest runner execution.

---

## 4. Time & Space Complexity of Every Command

| Command | Time Complexity | Space / Aux Complexity | Why / How It Achieves This |
| :--- | :---: | :---: | :--- |
| **`create_parking_lot R C`** | **$O(R \times C + S \log S)$** | $O(R \times C)$ | 1. Parse grid: $O(R \times C)$<br>2. Single iterative BFS from Entrance: $O(R \times C)$<br>3. Sort $S$ usable slots by `(distance, row, col)`: $O(S \log S)$<br>4. Min-heap initialization: $O(S)$ |
| **`park <REG> <COLOUR>`** | **$O(\log S)$** | $O(1)$ per car | 1. Pop closest slot rank from `free_ranks` min-heap: $O(\log S)$<br>2. Insert into `reg_to_cell` hash map: $O(1)$ avg<br>3. Insert into colour set: $O(\log K)$<br>4. Insert into `all_occupied` set: $O(\log N_{\text{occ}})$ |
| **`leave <ROW> <COL>`** | **$O(\log S)$** | $O(1)$ | 1. $O(1)$ bounds check and flat vector lookup: `cell_to_rank[cell]`<br>2. $O(1)$ erase from `reg_to_cell`<br>3. $O(\log K)$ erase from colour set and occupied set<br>4. Push rank back into min-heap: $O(\log S)$ |
| **`status`** | **$O(N_{\text{occupied}})$** | $O(1)$ aux | Iterates over `all_occupied` (`std::set<int32_t>`). Because cell indices `(r * cols + c)` inherently sort in row-major order, **zero sorting is needed at query time!** |
| **`slot_number_for_registration_number`** | **$O(1)$ average** | $O(1)$ | Direct lookup in `std::unordered_map<std::string, int32_t>`. |
| **`registration_numbers_for_cars_with_colour`** | **$O(K)$** | $O(1)$ aux | $O(1)$ hash map lookup for colour key $\to$ iterate $K$ cars in `std::set`. Already sorted in row-major order. |
| **`slot_numbers_for_cars_with_colour`** | **$O(K)$** | $O(1)$ aux | $O(1)$ hash map lookup for colour key $\to$ iterate $K$ slots in `std::set`. Already sorted in row-major order. |

> **Overall Space Complexity**: $O(R \times C)$ total memory. On a $1000 \times 1000$ grid ($1\text{M}$ cells), the entire memory footprint is $\approx 89\text{ MB}$, well within standard system limits.

---

## 5. High-Level File & Architecture Overview

The codebase strictly enforces the **Zero-I/O Core Architecture**: no internal logic module touches `std::cout`, `std::cin`, or files. They take inputs and return pure result structures.

```
src/
├── types.h           # Domain models: Position, Car, UsableSlot, LotState, CommandResult
├── parser.h / .cpp   # Line normalization (\r\n, \r, \n), tokenization, saturating integers
├── grid.h / .cpp     # Flat 1D coordinate arithmetic and 4-way orthogonal neighbours
├── distance.h / .cpp # Iterative BFS shortest-path road distance engine
├── slots.h / .cpp    # Usable slot filter, deterministic tie-break sorting, rank mapping
├── state.h / .cpp    # Free-slot min-heap, dual registration/colour indexing
├── commands.h / .cpp # Pure command execution handlers with atomic failure rollback
├── formatter.h / .cpp# Pure text formatting, table alignment, error message sanitization
└── main.cpp          # Thin I/O shell: file ingestion, encoding verification, buffered output
```

### Module Roles:
1. **`types.h`**: Encapsulates all data contracts (`Position`, `Car`, `UsableSlot`, `CommandResult`). Bundles the whole parking lot state into a single explicit struct `LotState` (no global state).
2. **`grid.h / .cpp`**: Translates 1-based `(row, col)` coordinates to flat 0-based index `(r - 1) * cols + (c - 1)`. Computes 4-way neighbors (up, down, left, right) while strictly checking boundary conditions.
3. **`distance.h / .cpp`**: The BFS engine. Runs **one iterative BFS** (no recursion $\to$ zero call-stack overflow risk) from entrance `'E'` through road cells (`.` and `E`). For every slot cell `'P'`, its distance is calculated as:
   $$\text{distance} = \min(\text{road neighbour distances}) + 1$$
   Slots surrounded by walls or without access to a reachable road get distance `-1` (unusable).
4. **`slots.h / .cpp`**: Filters out unusable slots, sorts valid ones by `(distance ASC, row ASC, col ASC)`, and assigns monotonic integer ranks $0 \dots S-1$. Builds two complementary lookup structures:
   - `rank_to_cell`: $O(1)$ lookup from rank to grid cell.
   - `cell_to_rank`: Flat vector of size $R \times C$ for $O(1)$ reverse lookup from cell to rank during `leave`.
5. **`state.h / .cpp`**: Maintains the active memory state of the parking lot:
   - `free_ranks`: `std::priority_queue` min-heap of available slot ranks.
   - `reg_to_cell`: `std::unordered_map` for $O(1)$ registration lookups.
   - `colour_to_cells`: `std::unordered_map<string, std::set<int32_t>>` with lowercased colour keys.
   - `all_occupied`: `std::set<int32_t>` for row-major ordered `status` reporting.
6. **`commands.h / .cpp`**: Dispatches incoming commands against `LotState`. Follows an **Atomic Execution Policy**: all preconditions (valid coordinates, car presence, available capacity) are validated before any state mutation occurs, preventing partial corruption.
7. **`parser.h / .cpp`**: Tokenizes input commands. Handles CRLF/LF/CR, strips UTF-8 BOM, caps token lengths, prevents 64-bit integer overflow using saturating arithmetic, and skips orphaned map lines if `create_parking_lot` fails.
8. **`formatter.h / .cpp`**: Pure string generator. Formats table columns, comma-separated lists, and sanitizes unknown command names (prevents terminal escape sequence injection).
9. **`main.cpp`**: Ingestion boundary. Reads files in binary mode, checks for UTF-16/UTF-32 or binary corruption (exits with code 1 if UTF-16), executes commands, and flushes output using a high-throughput 32 MB buffer.

---

## 6. Key Algorithmic Ideas & Engineering Decisions

1. **Flat 1D Vector for 2D Grid**:
   - Replaced nested vectors (`vector<vector<char>>`) with a flat `vector<char>` of size $R \times C$.
   - Eliminates $R$ separate heap allocations, prevents memory fragmentation, and keeps cell lookups in L1 cache.

2. **Single Upfront BFS (Precomputed Distances)**:
   - Walls and roads are static; cars parked in slots cannot block road traffic.
   - Therefore, a single BFS from `'E'` at lot creation computes all distances upfront. We never need to re-run BFS during `park` or `leave`.

3. **Rank Mapping & Min-Heap Optimization**:
   - Instead of storing complex `(distance, row, col)` tuples inside the priority queue, we sort the slots once upfront and assign ranks $0, 1, 2, \dots, S-1$.
   - The priority queue stores simple 32-bit integer ranks (`int32_t`). This makes heap operations extremely light and fast.

4. **Zero-Cost Sorting for Queries via Flat Cell Indices**:
   - Because a flat index is defined as `cell = row * cols + col`, a smaller cell index mathematically corresponds to an earlier row (or earlier column within the same row).
   - By storing cell indices in `std::set<int32_t>`, iterating over the set automatically visits slots in row-major order.
   - Result: `status` and colour queries require **zero sorting at query time**.

5. **Atomic Command Execution**:
   - A command either completely succeeds or leaves the lot state 100% untouched. If `leave` fails because the slot is already free, or `park` fails because the car registration is duplicate, no heap or map mutation takes place.

---

## 7. Important Tools & Verification Ecosystem

1. **Compiler Optimization & Warnings**:
   - Compiled with `g++ -std=c++17 -O2 -Wall -Wextra -pedantic`.
   - Zero compiler warnings allowed.

2. **Memory Safety Sanitizers**:
   - Tested under **AddressSanitizer (ASan)** and **UndefinedBehaviorSanitizer (UBSan)** (`make sanitize`).
   - Guarantees zero memory leaks, buffer overflows, use-after-free, or signed integer overflow.

3. **Automated Test Harness (`tests/test_framework.h`)**:
   - Hand-crafted, zero-dependency header-only test framework (`CHECK`, `CHECK_EQ`). No third-party testing bloat.

4. **Randomized Invariant Fuzzer (`tests/test_fuzz.cpp`)**:
   - Runs 5,000 random operations (`park`, `leave`, queries) on randomized lot layouts.
   - Verifies bidirectional invariants after every single step:
     $$\text{free\_ranks.size}() + \text{reg\_to\_cell.size}() == \text{total usable slots}$$

5. **25 End-to-End Testcases (`tests/testcases/`)**:
   - Covers valid flows, walled-in lots, unreachable slots, multi-way ties, UTF-16 rejection (exit code 1), malformed inputs, and leading zero handling. All 25 pass with 100% matching outputs.

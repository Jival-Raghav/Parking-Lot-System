import os
import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def set_cell_background(cell, hex_color):
    """Sets the background color of a table cell."""
    tcPr = cell._element.get_or_add_tcPr()
    shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{hex_color}"/>')
    tcPr.append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    """Sets internal padding for a cell (in twips, 20 twips = 1 pt)."""
    tcPr = cell._element.get_or_add_tcPr()
    tcMar = parse_xml(
        f'<w:tcMar {nsdecls("w")}>'
        f'<w:top w:w="{top}" w:type="dxa"/>'
        f'<w:bottom w:w="{bottom}" w:type="dxa"/>'
        f'<w:left w:w="{left}" w:type="dxa"/>'
        f'<w:right w:w="{right}" w:type="dxa"/>'
        f'</w:tcMar>'
    )
    tcPr.append(tcMar)

def set_table_borders(table, color="D3D3D3"):
    """Sets light gray borders for the entire table."""
    tblPr = table._element.xpath('w:tblPr')
    if tblPr:
        borders = parse_xml(
            f'<w:tblBorders {nsdecls("w")}>'
            f'<w:top w:val="single" w:sz="4" w:space="0" w:color="{color}"/>'
            f'<w:left w:val="none"/>'
            f'<w:bottom w:val="single" w:sz="6" w:space="0" w:color="{color}"/>'
            f'<w:right w:val="none"/>'
            f'<w:insideH w:val="single" w:sz="4" w:space="0" w:color="{color}"/>'
            f'<w:insideV w:val="none"/>'
            f'</w:tblBorders>'
        )
        tblPr[0].append(borders)

def build_docx(output_path):
    doc = Document()

    # Set page margins (1 inch around)
    for section in doc.sections:
        section.top_margin = Inches(0.8)
        section.bottom_margin = Inches(0.8)
        section.left_margin = Inches(0.8)
        section.right_margin = Inches(0.8)

    # Styles & Colors
    PRIMARY_COLOR = RGBColor(0x1F, 0x49, 0x7D)    # Deep Navy
    SECONDARY_COLOR = RGBColor(0x2E, 0x75, 0xB6)  # Blue
    TEXT_DARK = RGBColor(0x26, 0x26, 0x26)        # Off-Black
    GRAY_TEXT = RGBColor(0x59, 0x59, 0x59)        # Subdued gray
    CODE_COLOR = RGBColor(0x9C, 0x00, 0x06)       # Maroon code
    HEADER_BG = "1F497D"                          # Dark Navy Hex
    ALT_ROW_BG = "F2F4F8"                         # Very light bluish gray
    CALLOUT_BG = "EBF1F5"                         # Light ice blue

    # Base font setup
    style_normal = doc.styles['Normal']
    style_normal.font.name = 'Calibri'
    style_normal.font.size = Pt(10.5)
    style_normal.font.color.rgb = TEXT_DARK
    style_normal.paragraph_format.line_spacing = 1.15
    style_normal.paragraph_format.space_after = Pt(4)

    # Document Title
    p_title = doc.add_paragraph()
    p_title.paragraph_format.space_before = Pt(0)
    p_title.paragraph_format.space_after = Pt(2)
    run_title = p_title.add_run("Parking Lot Management System")
    run_title.font.name = 'Calibri'
    run_title.font.size = Pt(24)
    run_title.font.bold = True
    run_title.font.color.rgb = PRIMARY_COLOR

    # Subtitle
    p_sub = doc.add_paragraph()
    p_sub.paragraph_format.space_after = Pt(14)
    run_sub = p_sub.add_run("High-Level Interview Preparation Guide: Architecture, Complexities, C++, CMake & Testing")
    run_sub.font.name = 'Calibri'
    run_sub.font.size = Pt(12)
    run_sub.font.color.rgb = SECONDARY_COLOR
    run_sub.font.bold = True

    # Divider line
    p_div = doc.add_paragraph()
    p_div.paragraph_format.space_after = Pt(12)
    p_div_border = parse_xml(f'<w:pBdr {nsdecls("w")}><w:bottom w:val="single" w:sz="12" w:space="1" w:color="1F497D"/></w:pBdr>')
    p_div._element.get_or_add_pPr().append(p_div_border)

    def add_h1(text):
        h = doc.add_paragraph()
        h.paragraph_format.space_before = Pt(14)
        h.paragraph_format.space_after = Pt(4)
        h.paragraph_format.keep_with_next = True
        r = h.add_run(text)
        r.font.name = 'Calibri'
        r.font.size = Pt(15)
        r.font.bold = True
        r.font.color.rgb = PRIMARY_COLOR
        return h

    def add_h2(text):
        h = doc.add_paragraph()
        h.paragraph_format.space_before = Pt(10)
        h.paragraph_format.space_after = Pt(3)
        h.paragraph_format.keep_with_next = True
        r = h.add_run(text)
        r.font.name = 'Calibri'
        r.font.size = Pt(12.5)
        r.font.bold = True
        r.font.color.rgb = SECONDARY_COLOR
        return h

    def add_callout(quote_text, attribution=""):
        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = False
        cell = table.cell(0, 0)
        cell.width = Inches(6.8)
        set_cell_background(cell, CALLOUT_BG)
        set_cell_margins(cell, top=140, bottom=140, left=200, right=200)

        # left border only
        tcPr = cell._element.get_or_add_tcPr()
        borders = parse_xml(
            f'<w:tcBorders {nsdecls("w")}>'
            f'<w:left w:val="single" w:sz="24" w:space="0" w:color="1F497D"/>'
            f'<w:top w:val="none"/><w:right w:val="none"/><w:bottom w:val="none"/>'
            f'</w:tcBorders>'
        )
        tcPr.append(borders)

        cp = cell.paragraphs[0]
        cp.paragraph_format.space_after = Pt(2)
        cp.paragraph_format.line_spacing = 1.15
        r_quote = cp.add_run(f'"{quote_text}"')
        r_quote.font.italic = True
        r_quote.font.size = Pt(10)
        r_quote.font.color.rgb = PRIMARY_COLOR

        if attribution:
            ap = cell.add_paragraph()
            ap.paragraph_format.space_before = Pt(2)
            ap.paragraph_format.space_after = Pt(0)
            r_attr = ap.add_run(f'— {attribution}')
            r_attr.font.bold = True
            r_attr.font.size = Pt(9.5)
            r_attr.font.color.rgb = SECONDARY_COLOR

        # Spacer after table
        sp = doc.add_paragraph()
        sp.paragraph_format.space_before = Pt(0)
        sp.paragraph_format.space_after = Pt(4)

    # ----------------------------------------------------
    # SECTION 1: ELEVATOR PITCH
    # ----------------------------------------------------
    add_h1("1. The 30-Second Elevator Pitch")
    p_lead = doc.add_paragraph()
    p_lead.add_run("When the interviewer asks: ").font.italic = True
    p_lead.add_run('"Tell me about this project and what you built."').font.bold = True

    pitch_text = (
        "I built a high-performance, zero-dependency Parking Lot Management System in C++17. "
        "Unlike standard toy parking lots that allocate slots sequentially (1, 2, 3...), this system models a "
        "real-world 2D grid containing an entrance ('E'), roads ('.'), walls ('#'), and parking slots ('P').\n\n"
        "The core algorithmic challenge was dynamic nearest-slot allocation based on actual driving road distance "
        "from the entrance—not straight-line Euclidean distance—with deterministic tie-breaking by row and column.\n\n"
        "I engineered the system around a Zero-I/O Core Architecture where all domain logic is purely memory-efficient "
        "and isolated from stream I/O. The system exceeds requirements: it processes a 1,000,000-cell grid with "
        "100,000 mixed commands in just 0.65 seconds while maintaining zero memory leaks under AddressSanitizer."
    )
    add_callout(pitch_text, "Recommended Interview Answer")

    # ----------------------------------------------------
    # SECTION 2: WHY C++?
    # ----------------------------------------------------
    add_h1("2. Why C++? (The Interview Justification)")
    p_cpp_intro = doc.add_paragraph()
    p_cpp_intro.add_run("When asked why C++17 was selected over Java, Python, or Go, focus on these 4 architectural advantages:")

    reasons = [
        ("Cache Locality & Flat Memory Layout",
         "In Java or Python, a 2D grid becomes an array of pointers to row arrays or boxed objects (vector<vector<T>> / ArrayList<ArrayList<T>>), creating heavy pointer chasing and CPU cache misses. In C++, we flatten the entire 1,000 x 1,000 grid into a contiguous flat 1D vector (std::vector<char>). Cell (r, c) is accessed in a single multiply-add: (r-1)*cols + (c-1), maximizing L1/L2 cache line hits and SIMD vectorization."),
        ("Zero-Overhead Abstractions & Deterministic Memory (RAII)",
         "Garbage-collected languages (Java/Go/Python) suffer from unpredictable stop-the-world GC pauses that degrade p99/p99.9 latency under 100,000-command bursts. C++ uses RAII: memory is freed deterministically the instant an object goes out of scope, guaranteeing predictable sub-millisecond command execution."),
        ("Zero External Dependencies",
         "The entire codebase relies purely on the C++17 Standard Library (<vector>, <queue>, <unordered_map>, <set>). It requires no third-party libraries (Boost, GoogleTest, Abseil) and compiles into a single, self-contained native executable across Linux, macOS, and Windows."),
        ("Raw Benchmark Performance",
         "End-to-end execution of a 1,000,000-cell grid creation, BFS shortest-path exploration, and 100,000 mixed operations takes only 0.65 seconds (650 ms), compared to 5 to 15+ seconds in VM or interpreted environments.")
    ]

    for title, desc in reasons:
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(3)
        r_b = p.add_run(f"{title}: ")
        r_b.bold = True
        r_b.font.color.rgb = PRIMARY_COLOR
        p.add_run(desc)

    # ----------------------------------------------------
    # SECTION 3: WHAT IS CMAKE? (MAKE vs CMAKE)
    # ----------------------------------------------------
    add_h1("3. What is CMake? (Make vs. CMake Explained)")
    p_make_intro = doc.add_paragraph()
    p_make_intro.add_run(
        "A common interview topic for C++ systems is the build pipeline. Interviewers want to verify whether you understand the difference between a build tool and a meta-build generator."
    )

    # Comparison Table
    table_cmake = doc.add_table(rows=5, cols=3)
    table_cmake.alignment = WD_TABLE_ALIGNMENT.CENTER
    table_cmake.autofit = False
    set_table_borders(table_cmake)

    col_widths = [Inches(1.8), Inches(2.5), Inches(2.5)]
    headers = ["Aspect", "Make (Build Tool)", "CMake (Meta-Build Generator)"]

    hdr_cells = table_cmake.rows[0].cells
    for i, title in enumerate(headers):
        hdr_cells[i].width = col_widths[i]
        set_cell_background(hdr_cells[i], HEADER_BG)
        set_cell_margins(hdr_cells[i], top=120, bottom=120, left=140, right=140)
        p = hdr_cells[i].paragraphs[0]
        r = p.add_run(title)
        r.bold = True
        r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
        r.font.size = Pt(9.5)

    cmake_rows = [
        ("Role", "Executes compiler commands (g++) to build binary directly.", "Generates native build scripts (Makefiles, Ninja, VS Solutions). Does not compile directly."),
        ("Configuration File", "Makefile", "CMakeLists.txt"),
        ("Cross-Platform", "Platform-specific (primarily Unix / Linux / macOS / WSL).", "True cross-platform. Generates Visual Studio on Windows, Make/Ninja on Linux/macOS."),
        ("Project Role", "Provides instant 1-word commands for developers (make, make test, make sanitize).", "Provides professional IDE integration (CLion, VS Code, Visual Studio) and CTest runner.")
    ]

    for row_idx, data in enumerate(cmake_rows, start=1):
        cells = table_cmake.rows[row_idx].cells
        bg_col = ALT_ROW_BG if row_idx % 2 == 1 else "FFFFFF"
        for col_idx in range(3):
            cells[col_idx].width = col_widths[col_idx]
            set_cell_background(cells[col_idx], bg_col)
            set_cell_margins(cells[col_idx], top=80, bottom=80, left=120, right=120)
            p = cells[col_idx].paragraphs[0]
            p.paragraph_format.line_spacing = 1.1
            p.paragraph_format.space_after = Pt(0)
            r = p.add_run(data[col_idx])
            r.font.size = Pt(9)
            if col_idx == 0:
                r.bold = True
                r.font.color.rgb = PRIMARY_COLOR

    p_post_tbl = doc.add_paragraph()
    p_post_tbl.paragraph_format.space_before = Pt(4)
    p_post_tbl.add_run("Key Takeaway to say in Interview: ").font.bold = True
    p_post_tbl.add_run(
        '"CMake generates the build system; Make executes it. We provide both so Unix developers can simply type \'make\', while Windows / IDE developers can build cleanly via CMake."'
    )

    # ----------------------------------------------------
    # SECTION 4: TIME & SPACE COMPLEXITY
    # ----------------------------------------------------
    add_h1("4. Time & Space Complexity of Every Command")
    doc.add_paragraph(
        "Every operation was engineered to meet strict asymptotic guarantees. Here is the exact complexity cheat sheet:"
    )

    table_comp = doc.add_table(rows=8, cols=4)
    table_comp.alignment = WD_TABLE_ALIGNMENT.CENTER
    table_comp.autofit = False
    set_table_borders(table_comp)

    c_widths = [Inches(1.8), Inches(1.3), Inches(0.9), Inches(2.8)]
    c_headers = ["Command", "Time Complexity", "Aux Space", "Algorithmic Implementation Rationale"]

    for i, title in enumerate(c_headers):
        cell = table_comp.rows[0].cells[i]
        cell.width = c_widths[i]
        set_cell_background(cell, HEADER_BG)
        set_cell_margins(cell, top=120, bottom=120, left=120, right=120)
        p = cell.paragraphs[0]
        r = p.add_run(title)
        r.bold = True
        r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
        r.font.size = Pt(9)

    complexities = [
        ("create_parking_lot R C", "O(R x C + S log S)", "O(R x C)", "Grid parse O(RC) + single iterative BFS O(RC) + sort S usable slots O(S log S) + build min-heap O(S)."),
        ("park <REG> <COLOUR>", "O(log S)", "O(1)", "Pops min rank from free_ranks heap in O(log S). Inserts into hash map O(1) and colour set O(log K)."),
        ("leave <ROW> <COL>", "O(log S)", "O(1)", "Reverse slot rank lookup via flat cell_to_rank vector in O(1). Erases from indexes and pushes rank back to heap in O(log S)."),
        ("status", "O(N_occupied)", "O(1)", "Iterates all_occupied (std::set<int32_t>). Flat indices (r*cols + c) preserve row-major order: ZERO sorting at query time!"),
        ("slot_number_for_registration_number", "O(1) average", "O(1)", "Direct hash map lookup in reg_to_cell (std::unordered_map<string, int32_t>)."),
        ("registration_numbers_for_cars_with_colour", "O(K)", "O(1)", "Hash lookup for colour O(1), iterates K cars in std::set. Already ordered in row-major order."),
        ("slot_numbers_for_cars_with_colour", "O(K)", "O(1)", "Hash lookup for colour O(1), iterates K slots in std::set. Already ordered in row-major order.")
    ]

    for row_idx, data in enumerate(complexities, start=1):
        cells = table_comp.rows[row_idx].cells
        bg_col = ALT_ROW_BG if row_idx % 2 == 1 else "FFFFFF"
        for col_idx in range(4):
            cells[col_idx].width = c_widths[col_idx]
            set_cell_background(cells[col_idx], bg_col)
            set_cell_margins(cells[col_idx], top=80, bottom=80, left=100, right=100)
            p = cells[col_idx].paragraphs[0]
            p.paragraph_format.line_spacing = 1.1
            p.paragraph_format.space_after = Pt(0)
            r = p.add_run(data[col_idx])
            r.font.size = Pt(8.5)
            if col_idx == 0:
                r.bold = True
                r.font.color.rgb = PRIMARY_COLOR
            elif col_idx == 1:
                r.bold = True
                r.font.color.rgb = CODE_COLOR

    p_sp = doc.add_paragraph()
    p_sp.paragraph_format.space_before = Pt(4)
    r_sp = p_sp.add_run("Total Memory Space Complexity: ")
    r_sp.bold = True
    p_sp.add_run("O(R x C) total RAM. On a 1,000 x 1,000 grid (1,000,000 cells), total memory usage is ~89 MB, well within standard hardware budgets.")

    # ----------------------------------------------------
    # SECTION 5: ARCHITECTURE & MODULE OVERVIEW
    # ----------------------------------------------------
    add_h1("5. High-Level Architecture & File Responsibilities")
    p_arch = doc.add_paragraph()
    p_arch.add_run("The codebase is governed by the ").font.color.rgb = TEXT_DARK
    r_z = p_arch.add_run("Zero-I/O Core Architecture Principle")
    r_z.bold = True
    r_z.font.color.rgb = PRIMARY_COLOR
    p_arch.add_run(
        ": internal business logic modules NEVER touch std::cout, std::cin, or disk files. "
        "They receive input models and return pure result structures. All I/O is isolated in main.cpp, and all text rendering is isolated in formatter.cpp."
    )

    modules = [
        ("types.h", "Domain Models & State Bundle", "Defines core structs: Position, Car, UsableSlot, ErrorReason, and CommandResult. Bundles the complete lot state inside explicit struct LotState (zero global mutable state)."),
        ("grid.h / .cpp", "Flat 1D Grid Arithmetic", "Translates 1-based (row, col) coordinates to flat 0-based cell index: (r-1)*cols + (c-1). Calculates 4-way orthogonal neighbours (up, down, left, right) with strict boundary protection."),
        ("distance.h / .cpp", "BFS Shortest Path Engine", "Executes ONE iterative BFS from entrance 'E' through road cells ('.' and 'E'). Precomputes road distances in O(RC). Calculates slot distance as min(reachable road neighbours) + 1. Slots walled-off or unreachable receive distance -1."),
        ("slots.h / .cpp", "Slot Usability & Rank Mapping", "Filters usable slots (distance >= 0), sorts them once by (distance ASC, row ASC, col ASC), and assigns monotonic integer ranks 0..S-1. Builds rank_to_cell (vector) and cell_to_rank (flat vector) for O(1) two-way lookups."),
        ("state.h / .cpp", "Active Parking Lot State", "Manages runtime state: free_ranks min-heap (std::priority_queue), reg_to_cell hash map (std::unordered_map), colour_to_cells (std::unordered_map<string, set<int32_t>>), and all_occupied set."),
        ("commands.h / .cpp", "Pure Command Handlers", "Dispatches commands against LotState. Enforces Atomic Execution: validates all preconditions (bounds, car presence, duplicate reg, capacity) BEFORE mutating state. Zero partial corruption on failure."),
        ("parser.h / .cpp", "Streaming Tokenizer & Normalizer", "Normalizes line endings (\\r\\n, \\r, \\n), strips UTF-8 BOM, validates 256-byte token caps, rejects control characters, prevents 64-bit integer overflow via saturating parsing, and skips orphaned map lines if create fails."),
        ("formatter.h / .cpp", "Pure Text Rendering Layer", "The ONLY module formatting output text. Generates aligned status tables, comma-separated lists, and sanitizes unknown command names against terminal escape injection."),
        ("main.cpp", "Thin I/O Shell & Buffer Manager", "The ONLY module performing I/O. Ingests files in binary mode, inspects encoding (rejects UTF-16 with exit code 1), passes commands to the engine, and flushes output via a high-throughput 32 MB buffer.")
    ]

    for fname, role, desc in modules:
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(3)
        r_fn = p.add_run(f"{fname} ")
        r_fn.bold = True
        r_fn.font.color.rgb = PRIMARY_COLOR
        r_role = p.add_run(f"[{role}]: ")
        r_role.bold = True
        r_role.font.color.rgb = SECONDARY_COLOR
        p.add_run(desc)

    # ----------------------------------------------------
    # SECTION 6: KEY ALGORITHMIC IDEAS & DECISIONS
    # ----------------------------------------------------
    add_h1("6. Key Algorithmic Ideas & Engineering Decisions")
    doc.add_paragraph(
        "These are the high-level design patterns and algorithmic insights you should mention to impress senior interviewers:"
    )

    decisions = [
        ("1. Single Upfront BFS Traversal",
         "Why: Walls and roads are static. Parked cars occupy slots and never obstruct driving lanes. Therefore, road distances are invariant over the lifetime of the lot. By executing ONE iterative BFS during create_parking_lot, we precompute all distances in O(R x C) and never need to run BFS again during park or leave."),
        ("2. Monotonic Rank Mapping with Min-Heap",
         "Why: Sorting three criteria (distance, row, col) inside a heap on every operation is expensive. Instead, we sort all usable slots once upfront and map each slot to an integer rank (0 to S-1). The priority queue holds simple 32-bit integers (int32_t). Popping and pushing ranks is extremely fast (tens of nanoseconds)."),
        ("3. Zero-Cost Sorting via Flat Cell Indices",
         "Why: Flat index cell = row * cols + col is strictly monotonic with respect to row-major order (row ASC, col ASC). By storing cell indices in std::set<int32_t>, iteration naturally visits cells in exact row-major order. Consequently, status and colour lookups require ZERO sorting at query time."),
        ("4. Atomic Operations (No Partial Mutation)",
         "Why: If a leave command fails (e.g. invalid slot, wall, or already free), or a park command fails (e.g. duplicate registration), all validation checks run prior to touching the heap or hash maps. The state remains 100% clean and consistent."),
        ("5. Binary-Safe File Streaming & 32MB Flush",
         "Why: Standard std::cout << endl flushes the stream on every line, causing kernel syscall thrashing on 100,000 commands. We accumulate output in a 32 MB memory buffer and flush in chunks, cutting I/O latency from seconds to milliseconds.")
    ]

    for title, desc in decisions:
        p = doc.add_paragraph()
        p.paragraph_format.space_after = Pt(4)
        r_t = p.add_run(f"{title}\n")
        r_t.bold = True
        r_t.font.color.rgb = PRIMARY_COLOR
        p.add_run(desc)

    # ----------------------------------------------------
    # SECTION 7: TOOLS & VERIFICATION ECOSYSTEM
    # ----------------------------------------------------
    add_h1("7. Testing, Sanitizers & Verification Ecosystem")
    doc.add_paragraph(
        "Demonstrate to your interviewer that this was developed with production-grade engineering rigor:"
    )

    tools = [
        ("Compiler Flags", "g++ -std=c++17 -O2 -Wall -Wextra -pedantic", "Strict standards compliance; compiles with ZERO warnings."),
        ("Memory Sanitizers", "AddressSanitizer (ASan) & UBSan (-fsanitize=address,undefined)", "Guarantees zero memory leaks, buffer overruns, use-after-free, or undefined behavior across all runs."),
        ("Custom Test Harness", "tests/test_framework.h", "Lightweight, zero-dependency header-only framework (CHECK, CHECK_EQ) avoiding heavy external testing frameworks."),
        ("Randomized Fuzzer", "tests/test_fuzz.cpp (5,000 iterations)", "Random state mutations verifying strict bidirectional invariant: free_ranks.size() + reg_to_cell.size() == total usable slots."),
        ("25-Case Integration Suite", "tests/testcases/ (run_all.sh)", "Covers PDF sample byte-matching, walled-in 0-slot lots, multi-way tie-breaks, malformed arguments, leading zeros, and UTF-16 rejection (exit 1). 100% pass rate.")
    ]

    for tool_name, cmd, note in tools:
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(3)
        r_tn = p.add_run(f"{tool_name} ")
        r_tn.bold = True
        r_tn.font.color.rgb = PRIMARY_COLOR
        r_cmd = p.add_run(f"({cmd}): ")
        r_cmd.font.color.rgb = CODE_COLOR
        p.add_run(note)

    # Save document
    doc.save(output_path)
    print(f"Successfully generated DOCX at: {output_path}")

if __name__ == "__main__":
    out_file = r"c:\Users\jival\Desktop\jival\Latlong\INTERVIEW_GUIDE.docx"
    build_docx(out_file)

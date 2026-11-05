import os
import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml
from docx.oxml.ns import nsdecls

def set_cell_background(cell, hex_color):
    tcPr = cell._element.get_or_add_tcPr()
    shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{hex_color}"/>')
    tcPr.append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=140, right=140):
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

def build_master_docx(output_path):
    doc = Document()

    for section in doc.sections:
        section.top_margin = Inches(0.7)
        section.bottom_margin = Inches(0.7)
        section.left_margin = Inches(0.75)
        section.right_margin = Inches(0.75)

    PRIMARY = RGBColor(0x1F, 0x49, 0x7D)    # Navy
    SECONDARY = RGBColor(0x2E, 0x75, 0xB6)  # Slate Blue
    TEXT_DARK = RGBColor(0x26, 0x26, 0x26)  # Charcoal
    CODE_COLOR = RGBColor(0x9C, 0x00, 0x06) # Maroon
    HEADER_BG = "1F497D"
    ALT_ROW_BG = "F4F6F9"
    CALLOUT_BG = "EBF1F5"

    style_normal = doc.styles['Normal']
    style_normal.font.name = 'Calibri'
    style_normal.font.size = Pt(10)
    style_normal.font.color.rgb = TEXT_DARK
    style_normal.paragraph_format.line_spacing = 1.15
    style_normal.paragraph_format.space_after = Pt(3)

    # Title
    p_title = doc.add_paragraph()
    p_title.paragraph_format.space_before = Pt(0)
    p_title.paragraph_format.space_after = Pt(2)
    r_title = p_title.add_run("Parking Lot Management System")
    r_title.font.size = Pt(24)
    r_title.font.bold = True
    r_title.font.color.rgb = PRIMARY

    # Subtitle
    p_sub = doc.add_paragraph()
    p_sub.paragraph_format.space_after = Pt(10)
    r_sub = p_sub.add_run("Complete Master Interview Revision Manual: Architecture, Full End-to-End Workflow, All Exception Scenarios & Technical Rationale")
    r_sub.font.size = Pt(11.5)
    r_sub.font.bold = True
    r_sub.font.color.rgb = SECONDARY

    # Divider line
    p_div = doc.add_paragraph()
    p_div.paragraph_format.space_after = Pt(10)
    p_div_border = parse_xml(f'<w:pBdr {nsdecls("w")}><w:bottom w:val="single" w:sz="12" w:space="1" w:color="1F497D"/></w:pBdr>')
    p_div._element.get_or_add_pPr().append(p_div_border)

    def add_h1(text):
        h = doc.add_paragraph()
        h.paragraph_format.space_before = Pt(12)
        h.paragraph_format.space_after = Pt(3)
        h.paragraph_format.keep_with_next = True
        r = h.add_run(text)
        r.font.size = Pt(14)
        r.font.bold = True
        r.font.color.rgb = PRIMARY
        return h

    def add_h2(text):
        h = doc.add_paragraph()
        h.paragraph_format.space_before = Pt(8)
        h.paragraph_format.space_after = Pt(2)
        h.paragraph_format.keep_with_next = True
        r = h.add_run(text)
        r.font.size = Pt(11.5)
        r.font.bold = True
        r.font.color.rgb = SECONDARY
        return h

    def add_callout(quote_text, attribution=""):
        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = False
        cell = table.cell(0, 0)
        cell.width = Inches(7.0)
        set_cell_background(cell, CALLOUT_BG)
        set_cell_margins(cell, top=120, bottom=120, left=180, right=180)
        tcPr = cell._element.get_or_add_tcPr()
        borders = parse_xml(f'<w:tcBorders {nsdecls("w")}><w:left w:val="single" w:sz="24" w:space="0" w:color="1F497D"/><w:top w:val="none"/><w:right w:val="none"/><w:bottom w:val="none"/></w:tcBorders>')
        tcPr.append(borders)
        cp = cell.paragraphs[0]
        cp.paragraph_format.space_after = Pt(2)
        r = cp.add_run(f'"{quote_text}"')
        r.font.italic = True
        r.font.size = Pt(9.5)
        r.font.color.rgb = PRIMARY
        if attribution:
            ap = cell.add_paragraph()
            ap.paragraph_format.space_before = Pt(2)
            ap.paragraph_format.space_after = Pt(0)
            ra = ap.add_run(f'— {attribution}')
            ra.font.bold = True
            ra.font.size = Pt(9)
            ra.font.color.rgb = SECONDARY
        sp = doc.add_paragraph()
        sp.paragraph_format.space_after = Pt(2)

    # 1. ELEVATOR PITCH
    add_h1("1. The 30-Second Elevator Pitch")
    pitch = (
        "I built a high-performance, zero-dependency Parking Lot Management System in C++17. "
        "Unlike standard toy parking lots that allocate slots sequentially (1, 2, 3...), this system models a "
        "real-world 2D grid containing an entrance ('E'), roads ('.'), walls ('#'), and parking slots ('P').\n\n"
        "The core algorithmic challenge was dynamic nearest-slot allocation based on actual driving road distance "
        "from the entrance—not straight-line Euclidean distance—with deterministic tie-breaking by row and column.\n\n"
        "I engineered the system around a Zero-I/O Core Architecture where all domain logic is purely memory-efficient "
        "and isolated from stream I/O. The system exceeds benchmark requirements: it processes a 1,000,000-cell grid with "
        "100,000 mixed commands in just 0.65 seconds while maintaining zero memory leaks under AddressSanitizer."
    )
    add_callout(pitch, "Ready-to-Speak Interview Response")

    # 2. FULL WORKFLOW PIPELINE
    add_h1("2. Complete End-to-End Execution Workflow")
    doc.add_paragraph(
        "Here is the complete chronological path of how data moves from disk to standard output:"
    )

    flow_steps = [
        ("Step 1: Main Ingestion Shell (main.cpp)",
         "Reads input file in binary mode. Runs is_unsupported_encoding(): rejects UTF-16/UTF-32 immediately with exit code 1 to stderr. If UTF-8 BOM is present (EF BB BF), strips it with zero-copy view.remove_prefix(3)."),
        ("Step 2: Streaming Parsing & Error Containment (parser.cpp)",
         "Splits lines across CRLF/LF/CR without allocations. Tokenizes commands on arbitrary spaces/tabs. If a create command fails, activates orphan-swallowing to skip subsequent map lines. Detects early stops if a known command appears inside map lines."),
        ("Step 3: Top-Level Pipeline Orchestration (main.cpp)",
         "Instantiates LotState lot. Loops through parsed Command objects and calls parking::commands::execute(lot, cmd). It takes the returned pure CommandResult and passes it to parking::formatter::format_result(res, out_buffer)."),
        ("Step 4: Domain Execution & Guard Checks (commands.cpp)",
         "For each command, executes read-only guard clauses (state::is_usable_slot, state::is_slot_occupied, state::is_registration_parked) BEFORE mutating anything. On create, delegates map validation to grid.cpp, BFS shortest-path exploration to distance.cpp, and rank sorting to slots.cpp."),
        ("Step 5: State Mutation & Index Updates (state.cpp)",
         "When park succeeds: pops min rank from free_ranks min-heap in O(log S) and updates reg_to_cell, colour_to_cells, and all_occupied. When leave succeeds: updates all 3 indexes in O(1)/O(log K) and recycles the rank back into the min-heap in O(log S)."),
        ("Step 6: Presentation Rendering (formatter.cpp)",
         "Converts CommandResult into exact formatted strings, padded status tables, or sanitized error lines. Appends directly to out_buffer with zero printing."),
        ("Step 7: High-Throughput Output Flush (main.cpp)",
         "Accumulates text in out_buffer and flushes to stdout in 32 MB chunks using binary std::fwrite(), cutting kernel syscall thrashing from seconds to milliseconds.")
    ]

    for title, desc in flow_steps:
        p = doc.add_paragraph(style='List Bullet')
        r_t = p.add_run(f"{title}: ")
        r_t.bold = True
        r_t.font.color.rgb = PRIMARY
        p.add_run(desc)

    # 3. DETAILED COMMAND WORKFLOW TRACES
    add_h1("3. Step-by-Step Command Lifecycle Traces")
    
    add_h2("A. How 'create_parking_lot R C' Works Under the Hood:")
    c_steps = [
        "1. commands.cpp checks if lot.created == true. If so, returns LOT_ALREADY_EXISTS.",
        "2. grid::validate_map() checks that exactly R rows exist, each row has C columns, only valid chars {E, ., #, P} exist, and exactly ONE 'E' is present.",
        "3. commands.cpp builds LotState new_lot LOCALLY on the stack first (transactional safety).",
        "4. distance::compute_road_distances() runs ONE iterative BFS from 'E' through road cells ('.' and 'E'). Returns flat road_dist vector.",
        "5. slots::build_usable_slots() checks all 'P' cells. Computes slot distance = min(road neighbours) + 1. Rejects walled-in slots (d == -1).",
        "6. slots::build_usable_slots() sorts usable slots once using std::sort by (distance ASC, row ASC, col ASC), assigns integer ranks (0..S-1), and fills the free_ranks min-heap.",
        "7. commands.cpp commits: lot = std::move(new_lot). Returns usable slot count."
    ]
    for s in c_steps:
        doc.add_paragraph(s, style='List Bullet')

    add_h2("B. How 'park <REG> <COLOUR>' Works Under the Hood:")
    p_steps = [
        "1. commands.cpp verifies lot.created is true and arguments are valid strings.",
        "2. Calls state::is_registration_parked(lot, reg). If car already parked -> returns REGISTRATION_ALREADY_PARKED.",
        "3. Checks if lot.free_ranks.empty(). If empty -> returns ResultKind::LOT_FULL ('Sorry, parking lot is full').",
        "4. Calls state::park_car(lot, reg, colour):",
        "   - Pops lowest rank from free_ranks min-heap in O(log S).",
        "   - Converts rank to flat cell index via lot.rank_to_cell[rank].",
        "   - Inserts into reg_to_cell hash map in O(1) average.",
        "   - Inserts cell index into colour_to_cells[colour_lower] set in O(log K).",
        "   - Inserts cell index into all_occupied set in O(log N_occ).",
        "   - Returns Position{row, col} converted via grid::to_position()."
    ]
    for s in p_steps:
        doc.add_paragraph(s, style='List Bullet')

    add_h2("C. How 'leave <ROW> <COL>' Works Under the Hood:")
    l_steps = [
        "1. commands.cpp parses row and col, validating digits and coordinate boundaries (1 <= r <= rows, 1 <= c <= cols).",
        "2. Converts (r, c) to flat cell index using grid::to_cell_index(r, c, cols).",
        "3. Calls state::is_usable_slot(): checks cell_to_rank[cell] != -1. If false -> returns NOT_A_USABLE_SLOT (entrance, road, wall, or walled slot).",
        "4. Calls state::is_slot_occupied(): checks rank_occupant[rank].has_value(). If false -> returns SLOT_ALREADY_FREE.",
        "5. Calls state::free_slot(lot, cell_idx):",
        "   - Erases car from reg_to_cell, colour_to_cells, and all_occupied.",
        "   - Clears rank_occupant[rank] = std::nullopt.",
        "   - Pushes integer rank back into free_ranks min-heap in O(log S). Slot is recycled!",
        "6. Returns SLOT_IS_FREE."
    ]
    for s in l_steps:
        doc.add_paragraph(s, style='List Bullet')

    # 4. ALL EXCEPTION & ERROR SCENARIOS
    add_h1("4. Master Catalog of Exception & Error Scenarios")
    doc.add_paragraph(
        "Here is the complete reference of every adversarial or error scenario, how the system detects it, and what it produces:"
    )

    tbl_err = doc.add_table(rows=22, cols=3)
    tbl_err.alignment = WD_TABLE_ALIGNMENT.CENTER
    tbl_err.autofit = False
    set_table_borders(tbl_err)

    col_w = [Inches(1.8), Inches(2.7), Inches(2.5)]
    headers = ["Input Scenario", "Internal Detection & Handling", "Specification Output & Result"]
    for i, title in enumerate(headers):
        cell = tbl_err.rows[0].cells[i]
        cell.width = col_w[i]
        set_cell_background(cell, HEADER_BG)
        set_cell_margins(cell, top=100, bottom=100, left=100, right=100)
        p = cell.paragraphs[0]
        r = p.add_run(title)
        r.bold = True
        r.font.size = Pt(8.5)
        r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)

    scenarios = [
        ("Pre-creation command", "Guard check !lot.created triggers in commands.cpp", "Error: <cmd> failed: parking lot not created"),
        ("Invalid create dimensions (0, negative, decimals, words)", "Saturating parser rejects non-digits; orphan map skip activates", "Error: create_parking_lot failed: invalid arguments (orphaned map swallowed)"),
        ("Dimensions > 20,000,000 cells", "Checked before memory allocation to prevent OOM crash", "Error: create_parking_lot failed: grid too large"),
        ("Bad map characters (p, 1, e, X)", "grid::validate_map() rejects characters outside {E, ., #, P}", "Error: create_parking_lot failed: invalid map"),
        ("Row length mismatch (short / long)", "grid::validate_map() checks each line.size() == cols", "Error: create_parking_lot failed: invalid map"),
        ("Invalid entrance count (0 or 2+ E)", "grid::validate_map() checks entrance_count == 1", "Error: create_parking_lot failed: invalid map"),
        ("Declared rows > map rows (early stop)", "Parser sees known command (e.g. park) -> stops map consumption", "Error: create_parking_lot failed: invalid map, park processed next"),
        ("Declared rows < map rows", "Creates lot with declared rows; emits warning for excess rows", "Created a parking lot... then Error: unexpected map data ignored"),
        ("Blank line between create and map", "Parser breaks map sequence upon encountering empty line", "Error: create_parking_lot failed: invalid map"),
        ("Second create while lot exists", "Guard check lot.created triggers; old cars preserved", "Error: create_parking_lot failed: lot already exists"),
        ("Walled-in / unreachable slots", "BFS road distance is -1; compute_slot_distance returns -1", "Slot filtered out; if 0 slots: Created lot with 0 usable slots"),
        ("Park when lot is full", "free_ranks min-heap is empty", "Sorry, parking lot is full"),
        ("Duplicate car registration", "state::is_registration_parked() finds plate in reg_to_cell", "Error: park failed: registration already parked"),
        ("Malformed leave (words, decimals)", "Saturating number parser rejects invalid coordinate tokens", "Error: leave failed: invalid arguments"),
        ("Leave coordinate outside grid", "Checks r > lot.rows or c > lot.cols", "Error: leave failed: slot out of range"),
        ("Leave on road, wall, entrance", "state::is_usable_slot() returns false (cell_to_rank == -1)", "Error: leave failed: not a usable slot"),
        ("Leave on already-empty slot", "state::is_slot_occupied() returns false (no occupant)", "Error: leave failed: slot already free"),
        ("Unknown command (PARK, hello)", "Parser flags unknown token; name sanitized to printable ASCII", "Error: unknown command '<sanitized>'"),
        ("Missing / extra query args", "Arg count checked in commands.cpp", "Error: <cmd> failed: invalid arguments"),
        ("Plate or colour not found", "Hash lookup returns empty set or end iterator", "Not found"),
        ("UTF-16 / UTF-32 encoded file", "main.cpp checks binary BOM; exits with code 1", "Nothing on stdout; stderr: Error: unsupported file encoding")
    ]

    for row_idx, data in enumerate(scenarios, start=1):
        cells = tbl_err.rows[row_idx].cells
        bg_col = ALT_ROW_BG if row_idx % 2 == 1 else "FFFFFF"
        for col_idx in range(3):
            cells[col_idx].width = col_w[col_idx]
            set_cell_background(cells[col_idx], bg_col)
            set_cell_margins(cells[col_idx], top=70, bottom=70, left=80, right=80)
            p = cells[col_idx].paragraphs[0]
            p.paragraph_format.line_spacing = 1.05
            p.paragraph_format.space_after = Pt(0)
            r = p.add_run(data[col_idx])
            r.font.size = Pt(8)
            if col_idx == 0:
                r.bold = True
                r.font.color.rgb = PRIMARY
            elif col_idx == 2:
                r.font.color.rgb = CODE_COLOR

    # 5. CORE C++ ARCHITECTURAL PRINCIPLES
    add_h1("5. Core C++ Concepts to Articulate in the Interview")
    concepts = [
        ("Why 'const LotState& lot' in Checking Functions?",
         "Passing by const-reference means zero memory copying (0 nanoseconds), zero heap allocation, and a compile-time guarantee that checking functions cannot mutate state."),
        ("Why 'inline' in grid.h?",
         "Allows the compiler to substitute coordinate conversion math (r-1)*cols + (c-1) directly into calling code, completely eliminating CPU function call overhead."),
        ("Why Flat 1D Array instead of vector<vector<T>>?",
         "Eliminates pointer chasing and memory fragmentation. Keeps all 1,000,000 cells in contiguous memory for maximum L1/L2 cache locality."),
        ("Why std::sort on vector instead of std::set in slots.cpp?",
         "std::set allocates tree nodes on the heap for every element. std::sort on a contiguous std::vector is 10x faster due to sequential cache line prefetching."),
        ("Why Monotonic Rank Mapping with Min-Heap?",
         "Slots are sorted once upfront. The min-heap stores simple 32-bit integer ranks (int32_t). Popping and pushing ranks runs in O(log S) with minimal CPU cycles."),
        ("What is CMake vs. Make?",
         "Make is a build tool that executes a Makefile directly. CMake is a meta-build generator that creates native Makefiles or Visual Studio projects from CMakeLists.txt.")
    ]

    for title, desc in concepts:
        p = doc.add_paragraph()
        r_t = p.add_run(f"{title}\n")
        r_t.bold = True
        r_t.font.color.rgb = PRIMARY
        p.add_run(desc)

    doc.save(output_path)
    print(f"Master DOCX successfully written to: {output_path}")

if __name__ == "__main__":
    out_f = r"c:\Users\jival\Desktop\jival\Latlong\INTERVIEW_MASTER_REVISION_GUIDE.docx"
    build_master_docx(out_f)
    try:
        out_orig = r"c:\Users\jival\Desktop\jival\Latlong\INTERVIEW_GUIDE.docx"
        build_master_docx(out_orig)
    except Exception:
        pass

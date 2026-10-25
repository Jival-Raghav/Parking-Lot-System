# Parking Lot test inputs (25 files)

Run everything:  ./run_all.sh ./parking
(copy this folder next to your binary, or pass the binary path)

Under the error policy, each failed command prints ONE line:
  Error: <command> failed: <reason>
and the program keeps going. Exit code is 0 for every file except input23 (exit 1).

## A. Valid inputs (5) - exact expected output in expected/expectedN.txt
| File | What it covers |
|---|---|
| input1 | The PDF sample. Must match the PDF byte for byte. |
| input2 | Entrance walled in: 0 usable slots, full, empty status, all lookups Not found. |
| input3 | Equal-distance tie-break, unreachable slots, mixed-case colours, leave then re-park. |
| input4 | Slot touching several roads, slot reachable only through other slots, multi-way ties. |
| input5 | UTF-8 BOM, CRLF, tabs, repeated spaces, blank lines, trailing spaces, no final newline. |

## B. Error inputs (20) - what you should see
| File | Input problem | Expected behaviour |
|---|---|---|
| input6 | Commands before any create | 4 x "failed: parking lot not created", then a normal run after the valid create |
| input7 | create with a missing argument, orphan map follows | create fails (invalid arguments), orphan map lines silently skipped, later valid create works |
| input8 | Non-numeric dimensions | create fails, orphan map skipped, `park` before a lot fails, valid create works |
| input9 | 0, negative, decimal, extra and missing dims | 6 x "create_parking_lot failed: invalid arguments", then a normal run |
| input10 | Bad map characters (p, 1, e, X) | 4 x "create_parking_lot failed: invalid map", then a normal run |
| input11 | Row too long, row too short, empty row | 3 x invalid map, then a normal run |
| input12 | Rows and columns swapped (7 5 for a 5x7 map) | invalid map; the `park`/`status` after it are still processed (early stop) as "parking lot not created" |
| input13 | Declared rows (4) larger than real map (2) | invalid map; the following park/status lines are NOT eaten as map rows |
| input14 | Declared rows smaller than real map | create succeeds; ONE "unexpected map data ignored"; park/status work |
| input15 | Blank line between create line and map | invalid map; leftover map line silently skipped; `park` fails (no lot) |
| input16 | Spaces inside a row, leading spaces, tabs | 3 x invalid map, then a normal run |
| input17 | No E, then two E | 2 x invalid map, then a normal run |
| input18 | Second create while a lot exists | "lot already exists"; old cars still in status; new parks continue from old state |
| input19 | Huge or overflowing dimensions | "grid too large" / invalid arguments, with no huge allocation and no crash |
| input20 | 14 different malformed `leave` commands | each rejected with invalid arguments or out of range; state unchanged; `leave 02 01` succeeds (leading zeros allowed) |
| input21 | Semantic leave errors on a 1-usable-slot lot | out of range, entrance, road, wall, unusable P rejected; double leave says already free; no slot ever given to two cars |
| input22 | Wrong arg counts, wrong-case command names, unknown words, comments, duplicate registration | invalid arguments / unknown command / registration already parked; case-sensitive registration lookup gives Not found |
| input23 | UTF-16 file | message on STDERR, exit 1, nothing on stdout |
| input24 | NUL bytes, control character, 300-char token | affected commands rejected, binary-safe, no truncation, later valid commands work |
| input25 | File ends in the middle of the map, no final newline | park fails (no lot), create fails with invalid map, no crash |

## What to look for
- Any crash, hang (>10 s), sanitizer message or exit code other than listed = a bug.
- No error line should echo the bad input or mention line numbers.
- inputs 1-5: `diff -b` against expected/ (status column spacing is checked separately against the PDF).
- Leading zeros (input20), case-sensitive registrations (input22) and the early-stop / orphan-skip rules (inputs 7, 8, 12-15) are policy decisions from the change request. If your implementation chose differently, the README must say so.

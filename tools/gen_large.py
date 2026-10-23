#!/usr/bin/env python3
import sys

def generate_large_input(filename, rows=1000, cols=1000, num_commands=100000):
    with open(filename, 'w') as f:
        f.write(f"create_parking_lot {rows} {cols}\n")
        # Row 0: Entrance and roads
        f.write("E" + "." * (cols - 1) + "\n")
        # Alternate rows with P
        reg_row = "".join("P" if c % 2 == 1 else "." for c in range(cols))
        for _ in range(1, rows):
            f.write(reg_row + "\n")
        
        # Commands
        for i in range(num_commands // 2):
            f.write(f"park CAR-{i} White\n")
        for i in range(num_commands // 4):
            f.write(f"slot_number_for_registration_number CAR-{i}\n")
        for i in range(num_commands // 4):
            f.write(f"registration_numbers_for_cars_with_colour White\n")

if __name__ == "__main__":
    out_file = sys.argv[1] if len(sys.argv) > 1 else "large_input.txt"
    print(f"Generating large test input in '{out_file}'...")
    generate_large_input(out_file)
    print("Done!")

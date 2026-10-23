import os

os.makedirs("tests/bad_inputs", exist_ok=True)

test_cases = {
    "A1": (
        "create_parking_lot 5 3\nE..\n...\n..P\nstatus\n",
        "Error: create_parking_lot failed: invalid map\nError: status failed: parking lot not created\n"
    ),
    "A2": (
        "create_parking_lot 2 3\nE..\n..P\n...\n###\npark CAR-1 White\n",
        "Created a parking lot with 1 usable slots\nError: unexpected map data ignored\nAllocated slot: (2, 3)\n"
    ),
    "A3": (
        "create_parking_lot 3 2\nE..\n..P\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A4": (
        "create_parking_lot 2 2\n\nEP\n..\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A5": (
        "create_parking_lot 2 2\nE \n.P\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A6": (
        "create_parking_lot 2 2\n EP\n ..\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A7": (
        "create_parking_lot 2 2\nEp\n.1\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A8": (
        "create_parking_lot 3 3\nE..\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A9": (
        "create_parking_lot 3 3\nE....\n.\n..P\n",
        "Error: create_parking_lot failed: invalid map\n"
    ),
    "A10": (
        "create_parking_lot 2 2\n..\n.P\ncreate_parking_lot 2 2\nEE\n.P\n",
        "Error: create_parking_lot failed: invalid map\nError: create_parking_lot failed: invalid map\n"
    ),
    "A11": (
        "create_parking_lot 2 2\n..\n.P\ncreate_parking_lot 1 2\nEP\npark CAR-1 Red\n",
        "Error: create_parking_lot failed: invalid map\nCreated a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "A12": (
        "create_parking_lot 1 2\nEP\npark CAR-1 Red\ncreate_parking_lot abc def\nstatus\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\nError: create_parking_lot failed: invalid arguments\nSlot     Registration No    Colour\n(1, 2)   CAR-1              Red\n"
    ),
    "A13": (
        "create_parking_lot 1 2\nEP\npark CAR-1 Red\ncreate_parking_lot 1 2\nEP\nstatus\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\nError: create_parking_lot failed: lot already exists\nSlot     Registration No    Colour\n(1, 2)   CAR-1              Red\n"
    ),
    "A14": (
        "create_parking_lot abc def\nE..\n...\n..P\ncreate_parking_lot 1 2\nEP\npark CAR-1 Red\n",
        "Error: create_parking_lot failed: invalid arguments\nCreated a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "A15": (
        "create_parking_lot 1000000 1000000\nstatus\n",
        "Error: create_parking_lot failed: grid too large\nError: status failed: parking lot not created\n"
    ),
    "B1": (
        "create_parking_lot 1 2\nEP\nleave (1, 7)\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B2": (
        "create_parking_lot 1 2\nEP\nleave 1,7\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B3": (
        "create_parking_lot 1 7\nE.....P\npark CAR-1 White\nleave 01 07\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 7)\nSlot (1, 7) is free\n"
    ),
    "B4": (
        "create_parking_lot 1 7\nE.....P\nleave +1 7\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B5": (
        "create_parking_lot 2 3\nE..\n..P\nleave 2.5 3\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B6": (
        "create_parking_lot 1 2\nEP\nleave 0x5 1\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B7": (
        "create_parking_lot 1 2\nEP\nleave 1e3 1\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: invalid arguments\n"
    ),
    "B8": (
        "create_parking_lot 1 2\nEP\nleave 999999999999999999999999999999 1\n",
        "Created a parking lot with 1 usable slots\nError: leave failed: slot out of range\n"
    ),
    "B9": (
        "create_parking_lot 2 2\nEP\n.P\nleave 0 0\nleave -1 3\nleave abc 3\nleave 1\nleave 1 2 3\n",
        "Created a parking lot with 2 usable slots\nError: leave failed: invalid arguments\nError: leave failed: invalid arguments\nError: leave failed: invalid arguments\nError: leave failed: invalid arguments\nError: leave failed: invalid arguments\n"
    ),
    "C3": (
        "\xEF\xBB\xBFcreate_parking_lot 1 2\nEP\npark CAR-1 White\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "C4": (
        "create_parking_lot 1 2\r\nEP\r\npark CAR-1 White\r\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "C5": (
        "create_parking_lot 1 2\rEP\rpark CAR-1 White\r",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "C6": (
        "create_parking_lot 1 2\nE\0\npa\0rk CAR-1 White\n",
        "Error: create_parking_lot failed: invalid map\nError: unknown command 'pa?rk'\n"
    ),
    "C7": (
        "create_parking_lot 1 2\nEP\npark KA-\xC3\xA9-1234 R\xC3\xB8d\nstatus\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\nSlot     Registration No    Colour\n(1, 2)   KA-\xC3\xA9-1234         R\xC3\xB8d\n"
    ),
    "C8": (
        "create_parking_lot 1 2\nEP\npark " + ("A" * 257) + " Blue\n",
        "Created a parking lot with 1 usable slots\nError: park failed: invalid arguments\n"
    ),
    "C9": (
        "create_parking_lot 1 2\nEP\npark CAR\x01-1 Blue\n",
        "Created a parking lot with 1 usable slots\nError: park failed: invalid arguments\n"
    ),
    "C10": (
        "x" * (10 * 1024 * 1024),
        "Error: unknown command '" + ("x" * 32) + "...'\n"
    ),
    "C11": (
        "create_parking_lot 1 2\nEP\npark C1 Blue",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\n"
    ),
    "C12": (
        "   \n\n\t  \n\n",
        ""
    ),
    "D1": (
        "park C1 White\nleave 1 1\nstatus\nregistration_numbers_for_cars_with_colour White\nslot_numbers_for_cars_with_colour White\nslot_number_for_registration_number C1\n",
        "Error: park failed: parking lot not created\nError: leave failed: parking lot not created\nError: status failed: parking lot not created\nError: registration_numbers_for_cars_with_colour failed: parking lot not created\nError: slot_numbers_for_cars_with_colour failed: parking lot not created\nError: slot_number_for_registration_number failed: parking lot not created\n"
    ),
    "D2": (
        "foo\npark 1 2 3\nleave 0 0\nstatus bar\n",
        "Error: unknown command 'foo'\nError: park failed: parking lot not created\nError: leave failed: parking lot not created\nError: status failed: parking lot not created\n"
    ),
    "D3": (
        "create_parking_lot 2 2\nEP\n.P\npark C1 Red\npark C1 Blue\nleave 1 2\npark C1 Green\nstatus\n",
        "Created a parking lot with 2 usable slots\nAllocated slot: (1, 2)\nError: park failed: registration already parked\nSlot (1, 2) is free\nAllocated slot: (1, 2)\nSlot     Registration No    Colour\n(1, 2)   C1                 Green\n"
    ),
    "D4": (
        "create_parking_lot 2 2\nEP\n.P\npark C1 Red\nleave 1 2\nleave 1 2\npark C2 Blue\npark C3 Grey\nstatus\n",
        "Created a parking lot with 2 usable slots\nAllocated slot: (1, 2)\nSlot (1, 2) is free\nError: leave failed: slot already free\nAllocated slot: (1, 2)\nAllocated slot: (2, 2)\nSlot     Registration No    Colour\n(1, 2)   C2                 Blue\n(2, 2)   C3                 Grey\n"
    ),
    "D5": (
        "create_parking_lot 1 2\nEP\npark C1 Purple\nregistration_numbers_for_cars_with_colour Purple\nleave 1 2\nregistration_numbers_for_cars_with_colour Purple\n",
        "Created a parking lot with 1 usable slots\nAllocated slot: (1, 2)\nC1\nSlot (1, 2) is free\nNot found\n"
    )
}

for name, (in_data, out_data) in test_cases.items():
    with open(f"tests/bad_inputs/{name}.in", "wb") as f:
        f.write(in_data.encode("latin1"))
    with open(f"tests/bad_inputs/{name}.out", "wb") as f:
        f.write(out_data.encode("latin1"))

# Binary files C1 and C2 (exit 1 expected)
# C1: UTF-16
with open("tests/bad_inputs/C1.in", "wb") as f:
    f.write(b"\xff\xfe" + "create_parking_lot 1 2\nEP\n".encode("utf-16-le"))

# C2: UTF-32 / mostly-NUL file
with open("tests/bad_inputs/C2.in", "wb") as f:
    f.write(b"\xff\xfe\x00\x00" + b"\x00" * 500)

print("Generated bad_inputs test files!")

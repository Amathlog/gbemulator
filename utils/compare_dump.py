# Compare dumps between SameBoy and this emulator

from pathlib import Path
import sys
from typing import Tuple, List

file_sameboy = Path("/home/adrien/Programming/SameBoy/temp.txt")
file_us = Path("/home/adrien/Programming/gbemulator/build/src/exe/tmp.txt")

def fetch_line(line: str) -> Tuple[int, str]:
    address, inst = line.split(": ")
    address = int(address, 16)
    return address, inst

def advance_loop(sameboy_dump: List[str], sameboy_idx: int, us_dump: List[str], us_idx: int) -> Tuple[bool, int, int]:
    previous_address_sameboy, previous_inst_sameboy = fetch_line(sameboy_dump[sameboy_idx-1])
    previous_address_us, previous_inst_us = fetch_line(us_dump[us_idx-1])

    address_sameboy, inst_sameboy = fetch_line(sameboy_dump[sameboy_idx])
    address_us, inst_us = fetch_line(us_dump[us_idx])

    if previous_inst_sameboy.startswith("JR") and previous_inst_us.startswith("JR"):
        # negative jump, find the one that needs to advance
        if "-" in previous_inst_us:
            if address_sameboy - previous_address_sameboy > 0:
                # advance us
                while address_us != address_sameboy:
                    us_idx += 1
                    if us_idx >= len(us_dump):
                        return False, 0, 0
                    address_us, _ = fetch_line(us_dump[us_idx])
                return True, sameboy_idx, us_idx
            else:
                # advance us
                while address_us != address_sameboy:
                    sameboy_idx += 1
                    if sameboy_idx >= len(sameboy_dump):
                        return False, 0, 0
                    address_sameboy, _ = fetch_line(sameboy_dump[sameboy_idx])
                return True, sameboy_idx, us_idx
            
    return False, 0, 0


if __name__ == "__main__":
    if len(sys.argv) > 1:
        file_sameboy = Path(sys.argv[1])
        file_us = Path(sys.argv[2])

    with file_sameboy.open("r") as f:
        sameboy_dump = f.readlines()

    with file_us.open("r") as f:
        us_dump = f.readlines()

    start_address = "0100"
    for i in range(len(sameboy_dump)):
        if sameboy_dump[i][4:8] == start_address:
            sameboy_dump = sameboy_dump[i:]
            break

    sameboy_i, us_i = 0, 0
    while sameboy_i < len(sameboy_dump) and us_i < len(us_dump):
        sameboy_address, sameboy_inst = fetch_line(sameboy_dump[sameboy_i])
        us_address, us_inst = fetch_line(us_dump[us_i])

        if sameboy_address != us_address:
            # Try to advance
            success, new_sameboy_i, new_us_i = advance_loop(sameboy_dump, sameboy_i, us_dump, us_i)
            if not success:
                print(f"Found diff at line {sameboy_i} / {us_i}")
                print(f"Sameboy last 5 lines")
                print(*sameboy_dump[sameboy_i-5:sameboy_i+1])
                print(f"Us last 5 lines")
                print(*us_dump[us_i-5:us_i+1])
                break
            else:
                sameboy_i = new_sameboy_i
                us_i = new_us_i
        sameboy_i += 1
        us_i += 1

    
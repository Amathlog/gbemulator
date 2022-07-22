from ast import arg
from pathlib import Path
import sys
import subprocess
import os

ROOT_PATH = Path(os.path.abspath(__file__)).parent.parent.parent
HARDWARE_FILE_LOCATION = ROOT_PATH / "tests" / "asm"

if "win32" in sys.platform:
    EXE_PATH = ROOT_PATH / "external" / "rgbds_bin"
    RGBASM_EXE = EXE_PATH / "rgbasm.exe"
    RGBFIX_EXE = EXE_PATH / "rgbfix.exe"
    RGBLINK_EXE = EXE_PATH / "rgblink.exe"
else:
    EXE_PATH = ROOT_PATH / "build" / "external" / "rgbds" / "src"
    RGBASM_EXE = EXE_PATH / "rgbasm"
    RGBFIX_EXE = EXE_PATH / "rgbfix"
    RGBLINK_EXE = EXE_PATH / "rgblink"


def compile_file(file: Path) -> int:
    print(f"Compiling {file}...")
    if (not file.is_file() or file.suffix != ".asm"):
        print("Try to compile a dir or a file that doesn't have .asm as extension. Abort")
        return -1
    
    filename = file.stem
    object_file = (file.parent / filename).with_suffix(".o")
    rom_file = (file.parent / filename).with_suffix(".gb")

    compile_task = subprocess.run([RGBASM_EXE, "-L", "-i", HARDWARE_FILE_LOCATION, "-o", object_file, file])
    if (compile_task.returncode != 0):
        print("Compile failed")
        return compile_task.returncode

    link_task = subprocess.run([RGBLINK_EXE, "-o", rom_file, object_file])
    if (link_task.returncode != 0):
        print("Link failed")
        return link_task.returncode

    fix_task = subprocess.run([RGBFIX_EXE, "-v", "-p", "0xFF", rom_file])
    if (fix_task.returncode != 0):
        print("Fix failed")
        return fix_task.returncode

    print(f"Compile succeeded. Produced file {rom_file}")


def compile_all(dir: Path):
    all_files = [dir]
    asm_files = []
    while len(all_files) > 0:
        current = all_files.pop()
        
        if (current.is_file() and current.suffix == ".asm"):
            asm_files.append(current)
            continue

        if (current.is_dir()):
            all_files.extend(current.iterdir())

    print(f"Found {len(asm_files)} file to compile")
    return_code = 0
    for file in asm_files:
        if (compile_file(file) != 0):
            return_code = -1

    return return_code


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser("GameBoy ASM compiler")
    parser.add_argument("--all", action="store_true", default=False, help="Will iterate other the path provided")
    parser.add_argument("path", type=Path, nargs="*")

    args = parser.parse_args()

    if args.path is None or len(args.path) == 0:
        args.path = [Path(".")]

    func = compile_all if args.all else compile_file

    for path in args.path:
        func(path)

#!/usr/bin/env python3
"""
Script to generate .ai/context/files_old.md as a summary of all legacy source/header files.
Scans src/ and stored_src/ recursively for .h, .hh, .c, .cpp, .cc, .hpp files.
Outputs: .ai/docs/files_old.md as a markdown summary (all files uncategorized).
"""
import os
import pathlib

ROOTS = ["src"]
EXTS = {".h", ".hh", ".hpp", ".c", ".cpp", ".cc"}
OUTPUT = ".ai/context/files_old.md"

workspace = pathlib.Path(__file__).resolve().parents[2]
output_path = workspace / OUTPUT

files = []
for root in ROOTS:
    root_path = workspace / root
    for dirpath, _, filenames in os.walk(root_path):
        for fname in filenames:
            ext = pathlib.Path(fname).suffix.lower()
            if ext in EXTS:
                fpath = pathlib.Path(dirpath) / fname
                relpath = f"/{fpath.relative_to(workspace)}"
                try:
                    with open(fpath, 'r', encoding='utf-8', errors='ignore') as f:
                        lines = sum(1 for _ in f)
                except Exception:
                    lines = "?"
                files.append((relpath, lines))

with open(output_path, 'w', encoding='utf-8') as out:
    out.write("# Legacy Source File Inventory\n\n")
    out.write("## Uncategorized\n")
    for relpath, lines in sorted(files):
        out.write(f"* {relpath} ({lines} LOC)\n")
    out.write("\n---\n\nThis summary is auto-generated. Edit to add descriptions and categories as you document each file.\n")

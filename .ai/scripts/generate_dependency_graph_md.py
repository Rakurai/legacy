#!/usr/bin/env python3
"""
Script to generate .ai/docs/dependency_graph.md for legacy C/C++ codebase.
Parses ctags and cscope outputs to build a richer dependency map for LLM context.

- ctags: symbol definitions and locations
-- run with:  ctags -R --c-kinds=+p --fields=+l --extras=+q src/

- cscope: function calls, symbol references, file includes
-- find src/ -name '*.[ch]*' > .ai/context/internal/cscope.files
-- cscope -b -q -R -i .ai/context/internal/cscope.files

Outputs: .ai/context/dependency_graph.md
"""
import pathlib
import re
from collections import defaultdict
from datetime import datetime

# Paths
workspace = pathlib.Path(__file__).resolve().parents[2]
ctags_path = workspace / ".ai/context/internal/tags"
cscope_path = workspace / ".ai/context/internal/cscope.out"
output_path = workspace / ".ai/context/dependency_graph.md"

# --- Parse ctags ---
# Map: file -> set(symbols defined)
file_symbols = defaultdict(set)
# Map: symbol -> file
symbol_file = {}
if ctags_path.exists():
    with open(ctags_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            if line.startswith('!') or not line.strip():
                continue
            parts = line.strip().split('\t')
            if len(parts) < 3:
                continue
            symbol, file, *_ = parts
            file_symbols[file].add(symbol)
            symbol_file[symbol] = file

# --- Parse cscope ---
# cscope -L -[0-9] can be used for queries, but we parse the database directly for includes, calls, and references
# We'll use a simple approach: parse the cscope.out as text
includes = defaultdict(set)  # file -> set(files it includes)
called_funcs = defaultdict(set)  # file -> set(functions it calls)
ref_symbols = defaultdict(set)  # file -> set(symbols it references)

if cscope_path.exists():
    with open(cscope_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    # cscope.out is a binary file, but if you run `cscope -x` you get a text cross-ref
    # We'll check for cscope.out.xref as a fallback
    xref_path = workspace / "cscope.out.xref"
    if xref_path.exists():
        with open(xref_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                # Format: symbol \t file \t function \t line# \t type
                parts = line.strip().split('\t')
                if len(parts) < 5:
                    continue
                symbol, file, func, lineno, typ = parts
                if typ == 'i':  # include
                    includes[file].add(symbol)
                elif typ == 'c':  # function call
                    called_funcs[file].add(symbol)
                elif typ in {'d', 'e', 'r', 's', 't'}:  # definition, enum, reference, struct, typedef
                    ref_symbols[file].add(symbol)
else:
    print("Warning: cscope.out or cscope.out.xref not found. Only ctags data will be used.")

# --- Build dependency graph ---
all_files = set(file_symbols.keys()) | set(includes.keys()) | set(called_funcs.keys()) | set(ref_symbols.keys())

with open(output_path, 'w', encoding='utf-8') as out:
    out.write("# Legacy Source Dependency Graph\n\n")
    out.write("This file summarizes, for each file, its symbol definitions, includes, function calls, and symbol references.\n\n")
    for f in sorted(all_files):
        out.write(f"## {f}\n")
        # Symbols defined
        syms = sorted(file_symbols.get(f, []))
        if syms:
            out.write("**Defines symbols:**\n")
            for s in syms:
                out.write(f"- {s}\n")
        # Includes
        incs = sorted(includes.get(f, []))
        if incs:
            out.write("**Includes files:**\n")
            for inc in incs:
                out.write(f"- {inc}\n")
        # Calls
        calls = sorted(called_funcs.get(f, []))
        if calls:
            out.write("**Calls functions:**\n")
            for c in calls:
                out.write(f"- {c}\n")
        # References
        refs = sorted(ref_symbols.get(f, []))
        if refs:
            out.write("**References symbols:**\n")
            for r in refs:
                out.write(f"- {r}\n")
        out.write("\n")
    out.write("---\n\nGenerated on: {}\n".format(datetime.now().strftime('%Y-%m-%d %H:%M')))

# Utility and Foundation Header Batch Summary (src/include/)

This batch includes the following header files:
- Flags.hh
- String.hh
- Pooled.hh
- Format.hh

## Overview
These headers provide foundational utilities and abstractions used throughout the codebase for flag management, string handling, memory pooling, and type-safe formatting. They support the core entity and system headers by enabling efficient, safe, and expressive operations on common data types.

### Key Structures and Types
- **Flags**: Bitset-based class for managing flag fields, supporting bitwise operations, conversions, and queries.
- **String**: Extension of std::string with case-insensitive comparisons, string utilities, and legacy C-string compatibility.
- **Pooled<T>**: Template for object pooling, reducing allocation overhead for frequently used types.
- **Format**: Namespace with variadic template wrappers for printf-style formatting, supporting custom types and conversions.

### Key Functions and Operations
- Flags: Bitwise operations, flag queries, conversions to/from strings, and utility methods.
- String: Case-insensitive comparisons, prefix/infix/suffix checks, numeric checks, transformations, and legacy compatibility functions.
- Pooled: Custom new/delete operators, pool management, and destruction utilities.
- Format: Type-safe printf/sprintf/fprintf wrappers, string formatting, and builder utilities.

### Dependencies
- Used by core entity classes (e.g., Character, Object, Area) and many subsystems.
- Enable consistent and efficient handling of flags, strings, and memory across the codebase.

## Open Questions / Assumptions
- The full set of flag meanings and conventions may be defined elsewhere (e.g., constants.hh).
- String.hh is designed for both modern and legacy code compatibility.

---
Batch processed and summarized as part of the documentation generation workflow.

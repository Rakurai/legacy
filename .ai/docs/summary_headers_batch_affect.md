# Affect System Header Batch Summary (src/include/affect/)

This batch summarizes the header files in `src/include/affect/`, which define the core types, enums, and internal APIs for the affect (status effect) system in the legacy codebase.

## Files in this batch
- Affect.hh (181 LOC): Defines the Affect class and related structures for status effects. Central to the affect system, including list management, comparators, and accessors for characters, objects, and rooms.
- Type.hh (145 LOC): Declares the affect::type enum class, listing all possible affect types (e.g., buffs, debuffs, weapon flags, object prefixes/suffixes).
- affect_int.hh (22 LOC): Declares internal functions for manipulating lists of Affect objects. Intended for internal use within affect modules.
- affect_list.hh (25 LOC): Declares additional internal list operations for affects, including adding, removing, copying, deduplicating, and sorting.

## Key Structures and Concepts
- **Affect class**: Represents a status effect applied to a character, object, or room. Includes fields for type, duration, modifier, owner, and where the effect applies.
- **affect::type enum**: Enumerates all possible affect types, such as buffs, debuffs, object prefixes/suffixes, and special status flags.
- **List management**: Internal headers provide functions for adding, removing, copying, and deduplicating affects in lists, supporting efficient status effect management.
- **Comparators and accessors**: Utilities for comparing affects, accessing their properties, and integrating with other game systems.

## Dependencies
- These headers are referenced by core game logic, character, object, and room management code throughout the codebase.
- The dependency graph shows that these types are foundational for the status effect system and are used in both gameplay and internal mechanics.

## Open Questions / Assumptions
- Are all affect types in Type.hh actively used, or are some legacy/unused?
- Are there additional affect-related headers or implementation files outside this directory?

---

This batch summary provides a foundation for understanding the affect system’s type definitions and internal APIs. See the dependency graph for symbol relationships and usage throughout the codebase.

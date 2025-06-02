# Utility, Enum, and Serialization Header Batch Summary (src/include/)

This batch includes the following header files:
- GarbageCollectingList.hh
- Guild.hh
- JSON/cJSON.hh
- Location.hh

## Overview
These headers provide utility containers, enums, JSON helpers, and serialization logic for the codebase. They support memory management, player classification, data interchange, and unique room identification.

### Key Structures and Types
- **GarbageCollectingList**: Template container for managing lists of pointers to Garbage-derived objects, supporting garbage collection and custom iteration.
- **Guild**: Enum listing all player guilds (none, mage, cleric, thief, warrior, necromancer, paladin, bard, ranger).
- **JSON/cJSON.hh**: Inline helpers for working with cJSON objects, including string, boolean, number, and flag access, plus file reading.
- **Location**: Class for uniquely identifying rooms by coordinate, vnum/index, or string, with serialization and comparison.

### Key Functions and Operations
- GarbageCollectingList: Add, remove, and delete objects marked as garbage; custom iterator skips garbage.
- Guild: Player classification for gameplay logic.
- JSON/cJSON.hh: JSON parsing, field access, and file reading.
- Location: Serialization, deserialization, and comparison for room identification.

### Dependencies
- Used by memory management, player systems, data interchange, and world/room logic.

## Open Questions / Assumptions
- GarbageCollectingList.hh assumes all objects inherit from Garbage.
- Location.hh is central to room identification and may interact with many world systems.

---
Batch processed and summarized as part of the documentation generation workflow.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a ROM/Merc-derived MUD (Multi-User Dungeon) server written in C++14. The codebase implements a text-based multiplayer game with a sophisticated architecture for managing players, NPCs, objects, areas, combat, and scripted behaviors.

## Build Commands

### Main Build
```bash
cd src
make legacy        # Build the main executable
make clean         # Remove object files and dependencies
make cleaner       # Full clean including executable
make install       # Copy executable to ../bin (backs up existing)
```

### Tests
```bash
cd test
make all           # Build all test executables
make run           # Build and run all tests
make clean         # Clean test artifacts
```

### Running the Server
```bash
./startup [port]   # Default port is 3000, runs from area/ directory
```

The startup script handles automatic restarts and logging (logs go to log/ directory).

## Architecture Overview

### Core Game Systems

**Game Loop & World Management**
- `Game` (src/Game.cc): Static game controller, holds global state and provides access to World
- `World` (src/World.cc): Central manager for all game entities - characters, objects, rooms, areas
- `comm.cc`: Main game loop (`game_loop_unix`), network I/O, descriptor management
- Entry point: `main()` in comm.cc

**Character System**
- `Character` (src/Character.cc): Base class for all living entities (PCs and NPCs)
- `Player` (src/Player.cc): Player-specific data (stored when logged out)
- `MobilePrototype` (src/MobilePrototype.cc): Templates for NPCs loaded from area files
- Pooled memory allocation for performance (`Pooled<T>` template)

**Object System**
- `Object` (src/Object.cc): In-game items
- `ObjectPrototype` (src/ObjectPrototype.cc): Templates loaded from area files
- `ObjectValue` (src/ObjectValue.cc): Union for type-specific object properties
- Dynamic loot generation in `lootv2.cc` using template objects (vnums 24355-24379)

**World Structure**
- `Area` (src/Area.cc): Collections of rooms, objects, and mobiles
- `Room` (src/Room.cc): Individual locations with exits
- `RoomPrototype`, `ExitPrototype`: Templates for world generation
- `worldmap/`: Overworld map system with quadtree spatial indexing

**Connection & State Management**
- `Descriptor` (src/Descriptor.cc): Network connection wrapper
- `conn/State.hh` and `conn/*State.cc`: State machine for login/character creation
- States include: GetNameState, GetOldPassState, GetGuildState, RollStatsState, PlayingState, etc.

**Command Processing**
- `interp.cc`: Command interpreter, dispatches to DO_FUN functions
- `cmd_table` in interp.cc: Vector mapping command names to handler functions
- Command functions declared in `interp.hh` with `DECLARE_DO_FUN` macro
- Commands implemented across many files: act_*.cc, wiz_*.cc, etc.

**Affect System**
- `affect/Affect.cc`: Temporary effects on characters/objects/rooms
- `affect/affect_cache_array.cc`: Performance optimization for stat calculations
- Support for stacking, duration, and modification of attributes

**MobProg System**
- `MobProg.cc`, `mob_prog.cc`: Scripting language for NPC behaviors
- Triggers: SPEECH_PROG, FIGHT_PROG, DEATH_PROG, GREET_PROG, etc.
- Programs loaded from area files and executed by game engine

**Combat & Skills**
- `fight.cc`: Combat mechanics, damage calculation
- `Battle.cc`: Battle state management
- `skill/`: Skill system implementation
- `magic.cc`: Spell effects and casting

**Clans & Social**
- `Clan.cc`: Player organizations
- `channels.cc`: Communication channels
- `Social.cc`: Emotes and social commands
- `Auction.cc`: Global auction system

**Persistence**
- `save.cc`: Player save/load
- `sql.cc`: SQLite integration for persistent data
- Player files in player/ directory
- Area files in area/ directory

### Key Subsystems

**Memory Management**
- `Pooled<T>` template: Object pooling for frequently allocated types
- `GarbageCollectingList<T>`: Manages character list with deferred deletion
- `Garbage` base class: Mark objects for delayed cleanup

**Event System**
- `event/Dispatcher.cc`: Event queue for scheduled actions
- Used for delayed effects, timed events

**Utilities**
- `String.cc`: Custom string class with copy-on-write semantics
- `Flags.cc`: Bit flag management
- `Format.cc`: Printf-style formatting
- `random.cc`: Random number generation
- `argument.cc`: Command argument parsing

**Third-Party**
- `deps/cJSON/`: JSON parsing (used for copyover/serialization)

## Code Organization

The codebase uses a flat structure in src/ with subdirectories for specialized systems:
- `affect/`: Affect/buff system
- `conn/`: Connection state machine
- `event/`: Event dispatcher
- `skill/`: Skill definitions and tables
- `util/`: Utility classes
- `worldmap/`: Overworld mapping
- `gem/`: Gem/item enhancement system
- `JSON/`: JSON utilities

Headers are in `src/include/` with matching subdirectories.

## Important Patterns

**Header Conventions**
- Use `#pragma once` for include guards
- Forward declare classes when possible to minimize dependencies
- Main type declarations in include/, implementations in src/

**Flags System**
- `Flags` class uses bitfields (A-Z, a-z, 0-9) for state
- Defined as `constexpr Flags::Bit` constants
- Used extensively for: act flags, affect flags, item flags, room flags, etc.

**String Handling**
- Custom `String` class, not std::string
- Lightweight copy-on-write implementation
- Null-aware (can represent nullptr distinct from empty string)

**Memory Model**
- Characters, Descriptors, and other hot types use `Pooled<T>` for allocation
- Garbage collection through `Garbage` base class and `GarbageCollectingList`
- Manual memory management with explicit destructors

**Lookup Tables**
- Many systems use static lookup tables (see `tables.hh`, `const.cc`)
- Guild definitions, races, attacks, item types, etc. in `merc.hh`

## Development Workflow

**Adding Commands**
1. Declare in `interp.hh` with `DECLARE_DO_FUN(do_yourcommand)`
2. Add entry to `cmd_table` in `interp.cc`
3. Implement in appropriate file (act_*.cc for player actions, wiz_*.cc for immortal commands)

**Modifying Game Data**
- Area files in area/ directory use custom text format
- MobProgs embedded in area files or separate files
- Edit while server is offline or use online building commands

**Adding Affects**
- Define affect type in `affect/Type.hh`
- Add to affect_table in `affect/affect_table.cc`
- Implement application/removal logic if needed

**Database Changes**
- SQLite database: legacydb.dump
- Schema changes require migration planning

## Testing

Unit tests in test/ directory use Boost.Test framework:
- StringTest.cc: Tests String class
- FlagsTest.cc: Tests Flags class

Run tests with `make run` in test/ directory.

## Documentation

Doxygen is configured (Doxyfile) to generate API documentation:
```bash
doxygen Doxyfile
```
Output goes to `.ai/context/doxygen_output/` (gitignored).

## Compiler Flags

- C++14 standard (`-std=c++14`)
- Warnings: `-Wall -Winline -Wsign-compare`
- Defines: `-Dunix -DSQL -DDEBUG -DIPV6`
- Libraries: SQLite3, libm, libpng16

## Critical Dependencies

- SQLite3 (persistence)
- libpng16 (image generation for worldmap)
- Boost.Test (unit testing only)

## Port Configuration

Default port 3000, configurable via startup script argument or `Game::port` variable.

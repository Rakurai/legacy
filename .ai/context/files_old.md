# Legacy Source File Inventory

## Uncategorized
* /src/Area.cc (939 LOC)
* /src/Auction.cc (373 LOC)
* /src/Battle.cc (3 LOC)
* /src/Character.cc (285 LOC)
* /src/Descriptor.cc (12 LOC)
* /src/Disabled.cc (136 LOC)
* /src/Exit.cc (26 LOC)
* /src/ExitPrototype.cc (20 LOC)
* /src/Flags.cc (69 LOC)
* /src/Game.cc (139 LOC)
* /src/GameTime.cc (133 LOC)
* /src/JSON/cJSON.cc (54 LOC)
* /src/Logging.cc (62 LOC)
* /src/MobProg.cc (68 LOC)
* /src/MobilePrototype.cc (161 LOC)
* /src/Note.cc (1985 LOC)
* /src/Object.cc (396 LOC)
* /src/ObjectPrototype.cc (275 LOC)
* /src/ObjectValue.cc (51 LOC)
* /src/Player.cc (16 LOC)
* /src/QuestArea.cc (25 LOC)
* /src/Reset.cc (179 LOC)
* /src/Room.cc (146 LOC)
* /src/RoomID.cc (79 LOC)
* /src/RoomPrototype.cc (132 LOC)
* /src/Shop.cc (14 LOC)
* /src/String.cc (352 LOC)
* /src/War.cc (1478 LOC)
* /src/Weather.cc (146 LOC)
* /src/World.cc (306 LOC)
* /src/act.cc (484 LOC)
* /src/act_comm.cc (1956 LOC)
* /src/act_info.cc (5493 LOC)
* /src/act_move.cc (3306 LOC)
* /src/act_obj.cc (5383 LOC)
* /src/affect/affect.cc (400 LOC)
* /src/affect/affect_cache_array.cc (83 LOC)
* /src/affect/affect_char.cc (373 LOC)
* /src/affect/affect_list.cc (156 LOC)
* /src/affect/affect_obj.cc (156 LOC)
* /src/affect/affect_room.cc (150 LOC)
* /src/affect/affect_table.cc (232 LOC)
* /src/alias.cc (190 LOC)
* /src/area_handler.cc (605 LOC)
* /src/argument.cc (193 LOC)
* /src/attribute.cc (191 LOC)
* /src/bank.cc (444 LOC)
* /src/channels.cc (1823 LOC)
* /src/clan-edit.cc (560 LOC)
* /src/comm.cc (1880 LOC)
* /src/config.cc (806 LOC)
* /src/conn/BreakConnectState.cc (63 LOC)
* /src/conn/ClosedState.cc (13 LOC)
* /src/conn/ConfirmNewNameState.cc (46 LOC)
* /src/conn/ConfirmNewPassState.cc (55 LOC)
* /src/conn/CopyoverRecoverState.cc (12 LOC)
* /src/conn/GetAlignmentState.cc (81 LOC)
* /src/conn/GetDeityState.cc (57 LOC)
* /src/conn/GetGuildState.cc (52 LOC)
* /src/conn/GetMudExpState.cc (59 LOC)
* /src/conn/GetNameState.cc (225 LOC)
* /src/conn/GetNewPassState.cc (35 LOC)
* /src/conn/GetOldPassState.cc (168 LOC)
* /src/conn/GetRaceState.cc (66 LOC)
* /src/conn/GetSexState.cc (46 LOC)
* /src/conn/GetWeaponState.cc (32 LOC)
* /src/conn/PlayingState.cc (15 LOC)
* /src/conn/ReadIMOTDState.cc (20 LOC)
* /src/conn/ReadMOTDState.cc (24 LOC)
* /src/conn/ReadNewMOTDState.cc (171 LOC)
* /src/conn/RollStatsState.cc (62 LOC)
* /src/conn/State.cc (27 LOC)
* /src/const.cc (2724 LOC)
* /src/debug.cc (378 LOC)
* /src/departed.cc (129 LOC)
* /src/deps/cJSON/cJSON.c (765 LOC)
* /src/deps/cJSON/cJSON.h (155 LOC)
* /src/dispel.cc (249 LOC)
* /src/duel.cc (834 LOC)
* /src/edit.cc (1005 LOC)
* /src/effects.cc (855 LOC)
* /src/event/Dispatcher.cc (48 LOC)
* /src/fight.cc (6007 LOC)
* /src/file.cc (269 LOC)
* /src/find.cc (548 LOC)
* /src/flag.cc (892 LOC)
* /src/gem/gem.cc (199 LOC)
* /src/handler.cc (1572 LOC)
* /src/help.cc (718 LOC)
* /src/hunt.cc (358 LOC)
* /src/ignore.cc (182 LOC)
* /src/include/Actable.hh (7 LOC)
* /src/include/Area.hh (114 LOC) - Defines the Area class, representing a game area with rooms, objects, mobiles, vnum range, and metadata. Handles loading, updating, and resetting.
* /src/include/Auction.hh (31 LOC)
* /src/include/Battle.hh (20 LOC)
* /src/include/Character.hh (244 LOC) - Defines the Character class, representing a player or NPC. Includes stats, inventory, status flags, group/party info, and macros for attribute and permission checks.
* /src/include/Clan.hh (43 LOC)
* /src/include/DepartedPlayer.hh (18 LOC)
* /src/include/Descriptor.hh (46 LOC)
* /src/include/Disabled.hh (24 LOC)
* /src/include/Duel.hh (55 LOC)
* /src/include/EQSocket.hh (15 LOC)
* /src/include/Edit.hh (28 LOC)
* /src/include/Exit.hh (51 LOC)
* /src/include/ExitPrototype.hh (23 LOC)
* /src/include/ExtraDescr.hh (28 LOC)
* /src/include/Flags.hh (108 LOC) - Defines the Flags class, a bitset-based utility for managing flag fields (e.g., status, permissions) with bitwise operations and conversions to/from strings.
* /src/include/Format.hh (83 LOC) - Provides the Format namespace, which implements variadic template wrappers for printf-style formatting using C++ types and custom string/flag conversions.
* /src/include/Game.hh (32 LOC)
* /src/include/GameTime.hh (39 LOC)
* /src/include/Garbage.hh (11 LOC)
* /src/include/GarbageCollectingList.hh (77 LOC)
* /src/include/Guild.hh (16 LOC)
* /src/include/JSON/cJSON.hh (58 LOC)
* /src/include/Location.hh (114 LOC)
* /src/include/Logging.hh (27 LOC)
* /src/include/MobProg.hh (71 LOC)
* /src/include/MobProgActList.hh (23 LOC)
* /src/include/MobilePrototype.hh (65 LOC)
* /src/include/Note.hh (51 LOC)
* /src/include/Object.hh (89 LOC) - Defines the Object class, representing in-game items. Handles inventory, ownership, affects, and item properties such as type, weight, and value.
* /src/include/ObjectPrototype.hh (47 LOC)
* /src/include/ObjectValue.hh (84 LOC)
* /src/include/Player.hh (138 LOC)
* /src/include/Pooled.hh (84 LOC) - Defines the Pooled<T> template, providing object pooling for efficient memory management of frequently created/destroyed objects.
* /src/include/QuestArea.hh (27 LOC)
* /src/include/Reset.hh (37 LOC)
* /src/include/Room.hh (62 LOC) - Defines the Room class, representing a location in the game world. Manages exits, contents, affects, flags, and provides methods for character movement and room state.
* /src/include/RoomID.hh (49 LOC)
* /src/include/RoomPrototype.hh (44 LOC)
* /src/include/Sector.hh (25 LOC)
* /src/include/Shop.hh (25 LOC)
* /src/include/Social.hh (31 LOC)
* /src/include/StoredPlayer.hh (21 LOC)
* /src/include/String.hh (147 LOC) - Defines the String class, a std::string extension with case-insensitive comparisons, string utilities, and legacy C-string compatibility.
* /src/include/Tail.hh (29 LOC)
* /src/include/Vnum.hh (28 LOC)
* /src/include/War.hh (61 LOC)
* /src/include/Weather.hh (32 LOC)
* /src/include/World.hh (81 LOC)
* /src/include/act.hh (39 LOC)
* /src/include/affect/Affect.hh (181 LOC) - Summarized in summary_headers_batch_affect.md
* /src/include/affect/Type.hh (145 LOC) - Summarized in summary_headers_batch_affect.md
* /src/include/affect/affect_int.hh (22 LOC) - Summarized in summary_headers_batch_affect.md
* /src/include/affect/affect_list.hh (25 LOC) - Summarized in summary_headers_batch_affect.md
* /src/include/argument.hh (32 LOC)
* /src/include/channels.hh (13 LOC)
* /src/include/comm.hh (16 LOC)
* /src/include/conn/State.hh (79 LOC)
* /src/include/constants.hh (1387 LOC)
* /src/include/declare.hh (341 LOC)
* /src/include/dispel.hh (12 LOC)
* /src/include/event/Dispatcher.hh (25 LOC)
* /src/include/event/Handler.hh (20 LOC)
* /src/include/event/event.hh (17 LOC)
* /src/include/file.hh (18 LOC)
* /src/include/find.hh (24 LOC)
* /src/include/gem/gem.hh (51 LOC)
* /src/include/interp.hh (501 LOC)
* /src/include/lookup.hh (25 LOC)
* /src/include/lootv2.hh (110 LOC)
* /src/include/macros.hh (129 LOC)
* /src/include/magic.hh (189 LOC)
* /src/include/memory.hh (31 LOC)
* /src/include/merc.hh (241 LOC)
* /src/include/music.hh (21 LOC)
* /src/include/random.hh (12 LOC)
* /src/include/skill/Type.hh (245 LOC)
* /src/include/skill/skill.hh (39 LOC)
* /src/include/sql.hh (39 LOC)
* /src/include/tables.hh (197 LOC)
* /src/include/telnet.hh (83 LOC)
* /src/include/typename.hh (34 LOC)
* /src/include/util/Image.hh (32 LOC)
* /src/include/vt100.hh (16 LOC)
* /src/include/worldmap/Coordinate.hh (86 LOC)
* /src/include/worldmap/MapColor.hh (48 LOC)
* /src/include/worldmap/Quadtree.hh (100 LOC)
* /src/include/worldmap/Region.hh (43 LOC)
* /src/include/worldmap/Worldmap.hh (30 LOC)
* /src/interp.cc (988 LOC)
* /src/load_config.cc (37 LOC)
* /src/lookup.cc (226 LOC)
* /src/loot_tables.cc (1317 LOC)
* /src/lootv2.cc (556 LOC)
* /src/magic.cc (6892 LOC)
* /src/marry.cc (465 LOC)
* /src/memory.cc (17 LOC)
* /src/mob_commands.cc (672 LOC)
* /src/mob_prog.cc (1937 LOC)
* /src/mobiles.cc (182 LOC)
* /src/music.cc (357 LOC)
* /src/objstate.cc (184 LOC)
* /src/paint.cc (191 LOC)
* /src/quest.cc (1897 LOC)
* /src/random.cc (258 LOC)
* /src/remort.cc (580 LOC)
* /src/rmagic.cc (512 LOC)
* /src/save.cc (1944 LOC)
* /src/scan.cc (223 LOC)
* /src/set-stat.cc (2197 LOC)
* /src/skill/skill_table.cc (37 LOC)
* /src/skills.cc (1783 LOC)
* /src/social.cc (453 LOC)
* /src/special.cc (1373 LOC)
* /src/sqlite.cc (154 LOC)
* /src/storage.cc (157 LOC)
* /src/tables.cc (1143 LOC)
* /src/typename.cc (328 LOC)
* /src/update.cc (1570 LOC)
* /src/util/Image.cc (181 LOC)
* /src/wiz_admin.cc (17 LOC)
* /src/wiz_build.cc (348 LOC)
* /src/wiz_coder.cc (676 LOC)
* /src/wiz_gen.cc (4220 LOC)
* /src/wiz_quest.cc (1001 LOC)
* /src/wiz_secure.cc (1165 LOC)
* /src/worldmap/Coordinate.cc (18 LOC)
* /src/worldmap/Region.cc (157 LOC)
* /src/worldmap/Worldmap.cc (108 LOC)

---

This summary is auto-generated. Edit to add descriptions and categories as you document each file.

#pragma once

#include "String.hh"

namespace progs {

enum class Type {
	ACT_PROG,
	SPEECH_PROG,
	RAND_PROG,
	FIGHT_PROG,
	DEATH_PROG,
	HITPRCNT_PROG,
	ENTRY_PROG,
	GREET_PROG,
	ALL_GREET_PROG,
	GIVE_PROG,
	BRIBE_PROG,
	BUY_PROG,
	TICK_PROG,
	BOOT_PROG,
	RAND_AREA_PROG,
	CALL_PROG,
	QUEST_REQUEST_PROG,
	QUESTOR_HERE_PROG,

	DROP_PROG,
};

Type name_to_type(const String&);
const String type_to_name(Type);

} // namespace progs

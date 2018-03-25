#include "progs/prog_table.hh"

namespace progs {

// possible names for different variables, to build an allowed variable list for each prog type
const String mobv = "iIjkl"; // the mob itself
const String rndv = "rRJKL"; // random char in room
const String mstv = "bBfgh"; // mob's master
const String actv = "nNems"; // the actor
const String dobv = "oOa";   // direct object
const String iobv = "pPA";   // indirect object (act progs only)
const String vicv = "tTEMS"; // victim (act progs only)

const std::map<Type, prog_table_t> prog_table = {
	{ Type::ACT_PROG,       { "act_prog",       mobv + mstv + rndv + actv + dobv + iobv + vicv }},
	{ Type::SPEECH_PROG,    { "speech_prog",    mobv + mstv + rndv + actv}},
	{ Type::RAND_PROG,      { "rand_prog",      mobv + mstv + rndv }},
	{ Type::FIGHT_PROG,     { "fight_prog",     mobv + mstv + rndv + actv }},
	{ Type::DEATH_PROG,     { "death_prog",     mobv + mstv + rndv }},
	{ Type::HITPRCNT_PROG,  { "hitprcnt_prog",  mobv + mstv + rndv + actv }},
	{ Type::ENTRY_PROG,     { "entry_prog",     mobv + mstv + rndv }},
	{ Type::GREET_PROG,     { "greet_prog",     mobv + mstv + rndv + actv }},
	{ Type::ALL_GREET_PROG, { "all_greet_prog", mobv + mstv + rndv + actv }},
	{ Type::GIVE_PROG,      { "give_prog",      mobv + mstv + rndv + actv + dobv }},
	{ Type::BRIBE_PROG,     { "bribe_prog",     mobv + mstv + rndv + actv + dobv }},
	{ Type::BUY_PROG,       { "buy_prog",       mobv + mstv + rndv + actv }},
	{ Type::TICK_PROG,      { "tick_prog",      mobv + mstv + rndv }},
	{ Type::BOOT_PROG,      { "boot_prog",      mobv + mstv + rndv }},
	{ Type::RAND_AREA_PROG, { "rand_area_prog", mobv + mstv + rndv }},
	{ Type::CONTROL_PROG,   { "control_prog",   mobv + mstv + rndv + actv }},
	{ Type::DROP_PROG,      { "drop_prog",      dobv + rndv + actv }},
};

} // namespace progs

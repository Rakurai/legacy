#include "progs/prog_table.hh"

namespace progs {

const std::pair<String, data::Type> c_self("self",    data::Type::Character); // mobprogs
const std::pair<String, data::Type> o_self("self",    data::Type::Object);    // objprogs
const std::pair<String, data::Type> w_orld("world",   data::Type::World);
const std::pair<String, data::Type> r_room("room",    data::Type::Room);
const std::pair<String, data::Type> c_rand("random",  data::Type::Character);
const std::pair<String, data::Type> c_mast("master",  data::Type::Character);
const std::pair<String, data::Type> c_actr("actor",   data::Type::Character);
const std::pair<String, data::Type> o_dobj("obj",     data::Type::Object);
const std::pair<String, data::Type> o_iobj("ind_obj", data::Type::Object);
const std::pair<String, data::Type> c_vict("victim",  data::Type::Character);
const std::pair<String, data::Type> s_args("argument",data::Type::String);


const std::map<Type, prog_table_t> prog_table = {
	{ Type::ACT_PROG,       { "act_prog",       { c_self, w_orld, r_room, c_mast, c_rand, c_actr, o_dobj, o_iobj, c_vict }}},
	{ Type::SPEECH_PROG,    { "speech_prog",    { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::RAND_PROG,      { "rand_prog",      { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::FIGHT_PROG,     { "fight_prog",     { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::DEATH_PROG,     { "death_prog",     { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::HITPRCNT_PROG,  { "hitprcnt_prog",  { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::ENTRY_PROG,     { "entry_prog",     { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::GREET_PROG,     { "greet_prog",     { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::ALL_GREET_PROG, { "all_greet_prog", { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::GIVE_PROG,      { "give_prog",      { c_self, w_orld, r_room, c_mast, c_rand, c_actr, o_dobj                 }}},
	{ Type::BRIBE_PROG,     { "bribe_prog",     { c_self, w_orld, r_room, c_mast, c_rand, c_actr, o_dobj                 }}},
	{ Type::BUY_PROG,       { "buy_prog",       { c_self, w_orld, r_room, c_mast, c_rand, c_actr                         }}},
	{ Type::TICK_PROG,      { "tick_prog",      { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::BOOT_PROG,      { "boot_prog",      { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::RAND_AREA_PROG, { "rand_area_prog", { c_self, w_orld, r_room, c_mast, c_rand                                 }}},
	{ Type::CALL_PROG,      { "call_prog",      { c_self, w_orld, r_room, c_mast, c_rand, s_args                         }}},
	{ Type::DROP_PROG,      { "drop_prog",      { o_self, w_orld, r_room,         c_rand, c_actr                         }}},
	{ Type::QUEST_REQUEST_PROG, { "quest_request_prog", { c_self, w_orld, r_room, c_mast, c_rand, c_actr                 }}},
	{ Type::QUESTOR_HERE_PROG, { "questor_here_prog", { c_self, w_orld, r_room, c_mast, c_rand, c_actr                   }}},
};

} // namespace progs

#include "progs/symbols/declare.hh"
#include "progs/symbols/FunctionSymbol.hh"
#include "progs/symbols/templates.hh"
#include "progs/contexts/Context.hh"
#include "Character.hh"
#include "Object.hh"
#include "Game.hh"
#include "World.hh"
#include "Room.hh"
#include "random.hh"
#include "merc.hh"
#include "act.hh"
#include "Format.hh"
#include "affect/Affect.hh"
#include "MobilePrototype.hh"
#include "ObjectPrototype.hh"
#include "RoomPrototype.hh"
#include "find.hh"
#include "interp.hh"
#include "progs/symbols/function_helpers.hh"
#include "argument.hh"
#include "progs/triggers.hh"


namespace progs {
namespace symbols {

#define dt data::Type

const std::vector<fn_type> fn_table = {
	// name          return type    parent type    arg types

	// conversions, so we don't have to overload everything.  always upgrade,
	// str->char is ok, ambiguous char->str is not.
	{ "cv_to_room",  dt::Room,      dt::String,    {} }, // lookup a room by location (map/vnum)
	{ "cv_to_room",  dt::Room,      dt::Integer,   {} }, // lookup a room by vnum
	{ "cv_to_room",  dt::Room,      dt::Character, {} }, // lookup a room by character

	{ "cv_to_char",  dt::Character, dt::String,    {} }, // find a character in the $room
	{ "cv_to_obj",   dt::Object,    dt::String,    {} }, // find an object in the $room

	{ "cv_to_str",   dt::String,    dt::Integer,   {} }, // simple string convert

//	{ "cv_to_bool",  dt::Boolean,   dt::World,     {} },
//	{ "cv_to_int",   dt::Integer,   dt::World,     {} },

	// world accessors
	{ "time",        dt::String,    dt::World,     {} },
	{ "rand",        dt::Boolean,   dt::World,     { dt::Integer } },
	{ "get_room",    dt::Room,      dt::World,     { dt::Room } }, // lookup by convertible type
	{ "get_char",    dt::Character, dt::World,     { dt::String } }, // lookup by name
	{ "get_obj",     dt::Object,    dt::World,     { dt::String } }, // lookup by name

	// character accessors
	{ "keywords",    dt::String,    dt::Character, {} },
	{ "name",        dt::String,    dt::Character, {} },
	{ "title",       dt::String,    dt::Character, {} },
	{ "he_she",      dt::String,    dt::Character, {} },
	{ "him_her",     dt::String,    dt::Character, {} },
	{ "his_her",     dt::String,    dt::Character, {} },
	{ "is_pc",       dt::Boolean,   dt::Character, {} },
	{ "is_npc",      dt::Boolean,   dt::Character, {} },
	{ "is_good",     dt::Boolean,   dt::Character, {} },
	{ "is_evil",     dt::Boolean,   dt::Character, {} },
	{ "is_neutral",  dt::Boolean,   dt::Character, {} },
	{ "is_immort",   dt::Boolean,   dt::Character, {} },
	{ "is_fighting", dt::Boolean,   dt::Character, {} },
	{ "is_killer",   dt::Boolean,   dt::Character, {} },
	{ "is_thief",    dt::Boolean,   dt::Character, {} },
	{ "is_charmed",  dt::Boolean,   dt::Character, {} },
	{ "in_room",     dt::String,    dt::Character, {} },
	{ "position",    dt::Integer,   dt::Character, {} },
	{ "level",       dt::Integer,   dt::Character, {} },
	{ "hitprcnt",    dt::Integer,   dt::Character, {} },
	{ "sex",         dt::Integer,   dt::Character, {} },
	{ "guild",       dt::Integer,   dt::Character, {} },
	{ "gold",        dt::Integer,   dt::Character, {} },
	{ "vnum",        dt::Integer,   dt::Character, {} },
	{ "state",       dt::Integer,   dt::Character, { dt::String } },
	{ "is_carrying", dt::Boolean,   dt::Character, { dt::String } },
	{ "is_wearing",  dt::Boolean,   dt::Character, { dt::String } },
	{ "master",      dt::Character, dt::Character, {} },

	// character actions
	{ "load_obj",    dt::Object,    dt::Character, { dt::Integer } },
	{ "set_gold",    dt::Void,      dt::Character, { dt::Integer } },
	{ "set_silver",  dt::Void,      dt::Character, { dt::Integer } },
	{ "do",          dt::Void,      dt::Character, { dt::String } },
	{ "to_room",     dt::Void,      dt::Character, { dt::Room } },
	{ "cast",        dt::Void,      dt::Character, { dt::String } },
	{ "at",          dt::Void,      dt::Character, { dt::Room, dt::Void } },
	{ "kill",        dt::Void,      dt::Character, { dt::Character } },
	{ "junk",        dt::Void,      dt::Character, { dt::String } }, // first, could be all.something
	{ "junk",        dt::Void,      dt::Character, { dt::Object } }, // overloaded for specific obj
	{ "purge",       dt::Void,      dt::Character, {} }, // first, purge room (except for mob)
	{ "purge",       dt::Void,      dt::Character, { dt::Character } }, // overloaded for specific mob
	{ "purge",       dt::Void,      dt::Character, { dt::Object } }, // overloaded for specific obj
	{ "transfer",    dt::Void,      dt::Character, { dt::Character, dt::Room } },
	{ "invis",       dt::Void,      dt::Character, { dt::Boolean } },
	{ "become_pet",  dt::Void,      dt::Character, { dt::Character } },
	{ "call",        dt::Void,      dt::Character, { dt::String, dt::String } },
	{ "state",       dt::Void,      dt::Character, { dt::String, dt::String } },
	{ "quest_assign",dt::Void,      dt::Character, { dt::String } },
	{ "quest_progress",dt::Void,    dt::Character, { dt::String } },

	// object accessors
	{ "keywords",    dt::String,    dt::Object,    {} },
	{ "name",        dt::String,    dt::Object,    {} },
	{ "sdesc",       dt::String,    dt::Object,    {} },
	{ "ind_art",     dt::String,    dt::Object,    {} },
	{ "type",        dt::Integer,   dt::Object,    {} },
	{ "value0",      dt::Integer,   dt::Object,    {} },
	{ "value1",      dt::Integer,   dt::Object,    {} },
	{ "value2",      dt::Integer,   dt::Object,    {} },
	{ "value3",      dt::Integer,   dt::Object,    {} },
	{ "value4",      dt::Integer,   dt::Object,    {} },
	{ "vnum",        dt::Integer,   dt::Object,    {} },

	// object actions
	{ "to_room",     dt::Void,      dt::Object,    { dt::Room } },
	{ "to_char",     dt::Void,      dt::Object,    { dt::Character } },
	{ "echo",        dt::Void,      dt::Object,    { dt::String } }, // send to all in room
	{ "echo_near",   dt::Void,      dt::Object,    { dt::String } }, // send to surrounding rooms
	{ "purge",       dt::Void,      dt::Object,    {} }, // first, purge room (except for obj)
	{ "purge",       dt::Void,      dt::Object,    { dt::Character } }, // overloaded for specific mob
	{ "purge",       dt::Void,      dt::Object,    { dt::Object } }, // overloaded for specific obj
	{ "transfer",    dt::Void,      dt::Object,    { dt::Character, dt::Room } },

	// room accessors
	{ "name",        dt::String,    dt::Room,      {} },
	{ "vnum",        dt::Integer,   dt::Room,      {} },
	{ "get_char",    dt::Character, dt::Room,      { dt::String } }, // lookup by name
	{ "get_obj",     dt::Object,    dt::Room,      { dt::String } }, // lookup by name

	// room actions
	{ "load_mob",    dt::Character, dt::Room,      { dt::Integer } },
	{ "load_obj",    dt::Object,    dt::Room,      { dt::Integer } },
	{ "purge",       dt::Void,      dt::Room,      {} }, // first, purge room
	{ "purge",       dt::Void,      dt::Room,      { dt::Character } }, // overloaded for specific mob
	{ "purge",       dt::Void,      dt::Room,      { dt::Object } }, // overloaded for specific obj
	{ "transfer",    dt::Void,      dt::Room,      { dt::Character, dt::Room } },

	// echos
	{ "echo",        dt::Void,      dt::Character, { dt::String } },                // send to room except self
	{ "echo_to",     dt::Void,      dt::Character, { dt::Character, dt::String } }, // send to victim
	{ "echo_other",  dt::Void,      dt::Character, { dt::Character, dt::String } }, // send to room except self and victim
	{ "echo_near",   dt::Void,      dt::Character, { dt::String } },                // send to surrounding rooms
	{ "echo",        dt::Void,      dt::Object,    { dt::String } },                // send to room
	{ "echo_to",     dt::Void,      dt::Object,    { dt::Character, dt::String } }, // send to victim
	{ "echo_other",  dt::Void,      dt::Object,    { dt::Character, dt::String } }, // send to room except victim
	{ "echo_near",   dt::Void,      dt::Object,    { dt::String } },                // send to surrounding rooms
	{ "echo",        dt::Void,      dt::Room,      { dt::String } },                // send to room
	{ "echo_to",     dt::Void,      dt::Room,      { dt::Character, dt::String } }, // send to victim
	{ "echo_other",  dt::Void,      dt::Room,      { dt::Character, dt::String } }, // send to room except victim
	{ "echo_near",   dt::Void,      dt::Room,      { dt::String } },                // send to surrounding rooms
};

#undef dt

/* pattern for handling a new parent type
template <> template <>
Character * FunctionSymbol<Character *>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	throw Format::format("unhandled function '%s'", name);
}
*/

// *******************************
// Functions that return Character
// *******************************

// functions that access World, return Character
Character *
eval_delegate_world_char(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "get_char") {
		String str = deref<String>(arg_list[0].get(), context);
		String arg;
		int count = 0, number = number_argument(str, arg);

		if (arg.empty())
			return nullptr;

		for (auto ch : Game::world().char_list)
			if (context.can_see(ch)
			 && (ch->name.has_words(arg))
			 && ++count == number)
				return ch;

		return nullptr;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Character
template <> template <>
Character * FunctionSymbol<Character *>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "master") return ch->master;

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Room, return Character
template <> template <>
Character * FunctionSymbol<Character *>::
eval_delegate(Room *room, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "get_char") {
		return fn_helper_get_char(deref<String>(arg_list[0].get(), context), context, room);
	}

	if (name == "load_mob") {
		int vnum = deref<int>(arg_list[0].get(), context);
		MobilePrototype *proto = Game::world().get_mob_prototype(vnum);

		if (proto == nullptr)
			throw Format::format("bad vnum %d", vnum);

		Character *mob = create_mobile(proto);

		if (mob)
			char_to_room(mob, room);

		return mob;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access String, return Character
template <> template <>
Character * FunctionSymbol<Character *>::
eval_delegate(const String str, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "cv_to_char") {
		Room *room;
		context.get_var("room", &room);
		return fn_helper_get_char(str, context, room);
	}

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return Object
// *******************************

// functions that access World, return Object
Object *
eval_delegate_world_obj(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "get_obj") {
		String str = deref<String>(arg_list[0].get(), context);
		String arg;
		int count = 0, number = number_argument(str, arg);

		if (arg.empty())
			return nullptr;

		for (Object *obj = Game::world().object_list; obj; obj = obj->next)
			if (context.can_see(obj)
			 && (obj->name.has_words(arg))
			 && ++count == number)
				return obj;

		return nullptr;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Object
template <> template <>
Object * FunctionSymbol<Object *>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "load_obj") {
		int vnum = deref<int>(arg_list[0].get(), context);
		ObjectPrototype *proto = Game::world().get_obj_prototype(vnum);

		if (proto == nullptr)
			throw Format::format("bad vnum %d", vnum);

		Object *obj = create_object(proto, 0);

		if (obj) {
			if (CAN_WEAR(obj, ITEM_TAKE))
				obj_to_char(obj, ch);
			else
				obj_to_room(obj, ch->in_room);
		}

		return obj;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Room, return Object
template <> template <>
Object * FunctionSymbol<Object *>::
eval_delegate(Room *room, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "get_obj") {
		String str = deref<String>(arg_list[0].get(), context);
		String arg;
		int count = 0, number = number_argument(str, arg);

		if (arg.empty())
			return nullptr;

		for (Object *obj = room->contents; obj; obj = obj->next_content)
			if (context.can_see(obj)
			 && (obj->name.has_words(arg))
			 && ++count == number)
				return obj;

		return nullptr;
	}

	if (name == "load_obj") {
		int vnum = deref<int>(arg_list[0].get(), context);
		ObjectPrototype *proto = Game::world().get_obj_prototype(vnum);

		if (proto == nullptr)
			throw Format::format("bad vnum %d", vnum);

		Object *obj = create_object(proto, 0);

		if (obj)
			obj_to_room(obj, room);

		return obj;
	}

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return Room
// *******************************

// functions that access World, return Room
Room *
eval_delegate_world_room(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "get_room") return deref<Room *>(arg_list[0].get(), context);

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Room
template <> template <>
Room * FunctionSymbol<Room *>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "cv_to_room") return ch->in_room;

	throw Format::format("unhandled function '%s'", name);
}

// functions that access String, return Room
template <> template <>
Room * FunctionSymbol<Room *>::
eval_delegate(const String str, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "cv_to_room") {
		Location location(str);
		Room *room;

		if (!location.is_valid()
		 || (room = Game::world().get_room(location)) == nullptr)
			throw Format::format("get_room(): bad location '%s'", str);

		return room;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Boolean, return Room
template <> template <>
Room * FunctionSymbol<Room *>::
eval_delegate(bool val, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	throw Format::format("unhandled function '%s'", name);
}

// functions that access Integer, return Room
template <> template <>
Room * FunctionSymbol<Room *>::
eval_delegate(int num, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "cv_to_room") {
		Location location((Vnum(num)));
		Room *room;

		if (!location.is_valid()
		 || (room = Game::world().get_room(location)) == nullptr)
			throw Format::format("bad location '%d'", num);

		return room;
	}

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return String
// *******************************

// functions that access World, return String
String
eval_delegate_world_str(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "time")    return Format::format("%d", Game::world().time.hour);

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return String
template <> template <>
String FunctionSymbol<String>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	bool can_see = context.can_see(ch);

	if (name == "keywords") {
		return ch->name;
	}

	if (name == "name") {
		if (!can_see)     return "someone";
		if (ch->is_npc()) return ch->name.lsplit().capitalize();
		                  return ch->name;
	}

	if (name == "title") {
		if (!can_see)     return "someone";
		if (ch->is_npc()) return ch->short_descr;
		                  return ch->name + " " + ch->pcdata->title;
	}

	if (name == "he_she") {
		static const char *he_she  [] = { "it", "he",  "she" };
		if (!can_see)     return "someone";
		                  return he_she[GET_ATTR_SEX(ch)];
	}

	if (name == "him_her") {
		static const char *him_her [] = { "it", "him", "her" };
		if (!can_see)     return "someone";
		                  return him_her[GET_ATTR_SEX(ch)];
	}

	if (name == "his_her") {
		static const char *his_her [] = { "its","his", "her" };
		if (!can_see)     return "someone's";
		                  return his_her[GET_ATTR_SEX(ch)];
	}

	if (name == "in_room") return ch->in_room->location.to_string();

	if (name == "state") {
		String key = deref<String>(arg_list[0].get(), context);
		return ch->state.get_str(key);
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Object, return String
template <> template <>
String FunctionSymbol<String>::
eval_delegate(Object *obj, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	bool can_see = context.can_see(obj);

	if (name == "keywords") {
		return obj->name;
	}

	if (name == "name") {
		if (!can_see)     return "something";
		                  return obj->name.lsplit();
	}

	if (name == "sdesc") {
		if (!can_see)     return "something";
		                  return obj->short_descr;
	}

	if (name == "ind_art") {
		char c = tolower(obj->name[0]);

		if (can_see && (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'))
			return "an";

		return "a";
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Room, return String
template <> template <>
String FunctionSymbol<String>::
eval_delegate(Room *room, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	bool can_see = context.can_see(room);

	if (name == "name") {
		if (!can_see)     return "somewhere";
		                  return room->name();
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Integer, return String
template <> template <>
String FunctionSymbol<String>::
eval_delegate(int num, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "cv_to_str") return Format::format("%d", num);

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return Boolean
// *******************************

// functions that access World, return Boolean
bool
eval_delegate_world_bool(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "rand")    return number_percent() <= deref<int>(arg_list[0].get(), context);

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Boolean
template <> template <>
bool FunctionSymbol<bool>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "is_pc")       return !ch->is_npc();
	if (name == "is_npc")      return ch->is_npc();
	if (name == "is_good")     return IS_GOOD(ch);
	if (name == "is_evil")     return IS_EVIL(ch);
	if (name == "is_neutral")  return IS_NEUTRAL(ch);
	if (name == "is_immort")   return IS_IMMORTAL(ch);
	if (name == "is_fighting") return ch->fighting != nullptr;
	if (name == "is_killer")   return IS_KILLER(ch);
	if (name == "is_thief")    return IS_THIEF(ch);
	if (name == "is_charmed")  return affect::exists_on_char(ch, affect::type::charm_person);
	if (name == "is_carrying") return get_obj_carry(ch, deref<String>(arg_list[0].get(), context)) != nullptr;
	if (name == "is_wearing")  return get_obj_wear(ch, deref<String>(arg_list[0].get(), context)) != nullptr;

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return Integer
// *******************************

// functions that access World, return Integer
int
eval_delegate_world_int(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Integer
template <> template <>
int FunctionSymbol<int>::
eval_delegate(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "position")   return ch->position;
	if (name == "level")      return ch->level;
	if (name == "guild")      return ch->guild;
	if (name == "gold")       return ch->gold;
	if (name == "vnum")       return ch->is_npc() ? ch->pIndexData->vnum.value() : 0;
	if (name == "sex")        return GET_ATTR_SEX(ch);
	if (name == "hitprcnt")   return ch->hit / GET_MAX_HIT(ch);

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Object, return Integer
template <> template <>
int FunctionSymbol<int>::
eval_delegate(Object *obj, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "type")      return obj->item_type;
	if (name == "value0")    return obj->value[0];
	if (name == "value1")    return obj->value[1];
	if (name == "value2")    return obj->value[2];
	if (name == "value3")    return obj->value[3];
	if (name == "value4")    return obj->value[4];
	if (name == "vnum")      return obj->pIndexData->vnum.value();

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Room, return Integer
template <> template <>
int FunctionSymbol<int>::
eval_delegate(Room *room, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "vnum")      return room->prototype.vnum.value();

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Functions that return Void
// *******************************

// functions that access World, return Void
void
eval_delegate_world_void(const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	throw Format::format("unhandled function '%s'", name);
}

// template class for void delegates
template <typename T>
void
eval_delegate_void(T, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context&) {
	throw Format::format("unhandled function '%s'", name);
}

// functions that access Character, return Void
template <>
void
eval_delegate_void(Character *ch, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "set_gold") {
		ch->gold = deref<int>(arg_list[0].get(), context);
		return;
	}

	if (name == "set_silver") {
		ch->silver = deref<int>(arg_list[0].get(), context);
		return;
	}

	if (name == "do") {
		String argument = deref<String>(arg_list[0].get(), context);
		interpret(ch, argument);
		return;
	}
	
	if (name == "to_room") {
		Room *room = deref<Room *>(arg_list[0].get(), context);

		if (room == nullptr)
			throw Format::format("room is null");

		if (ch->fighting != nullptr)
			stop_fighting(ch, true);

		char_from_room(ch);
		char_to_room(ch, room);
		return;
	}
	
	if (name == "echo") {
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo(buf, ch, nullptr, nullptr, ch->in_room);
		return;
	}

	if (name == "echo_to") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_to(buf, ch, nullptr, victim, ch->in_room);
		return;
	}

	if (name == "echo_other") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_other(buf, ch, nullptr, victim, ch->in_room);
		return;
	}
	
	if (name == "echo_near") {
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo_near(buf, ch, nullptr, nullptr, ch->in_room);
		return;
	}
	
	if (name == "cast") {
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		do_mpcast(ch, buf);
		return;
	}
	
	if (name == "at") {
		Room *room = deref<Room *>(arg_list[0].get(), context);

		if (room == nullptr)
			throw Format::format("room is null");

		Room *old_room = ch->in_room;

		char_from_room(ch);
		char_to_room(ch, room);

		deref<int>(arg_list[1].get(), context); // execute

		if (!ch->is_garbage()) {
			char_from_room(ch);
			char_to_room(ch, old_room);
		}

		return;
	}

	if (name == "kill") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);

		if (victim == nullptr)
			throw Format::format("victim is null");

		if (ch->in_room != victim->in_room)
			throw Format::format("victim not in same room");

		if (affect::exists_on_char(ch, affect::type::charm_person)
		 && ch->master == victim)
			throw Format::format("charmed mob attacking master");

		if (ch->fighting)
			throw Format::format("already fighting");

		multi_hit(ch, victim, skill::type::unknown);
		return;
	}

	if (name == "junk") {
		// overloaded, could be Object or String
		if (arg_list[0]->type == data::Type::Object) {
			fn_helper_junk(ch, deref<Object *>(arg_list[0].get(), context));
			return;
		}

		const String args = deref<String>(arg_list[0].get(), context);
		String arg = args.lsplit();

		if (arg != "all" && !arg.has_prefix("all.")) {
			Object *obj;

			if ((obj = get_obj_wear(ch, arg)) != nullptr
			 || (obj = get_obj_carry(ch, arg)) != nullptr)
				fn_helper_junk(ch, obj);
		}
		else {
			String word;
			if (arg.length() > 4) // all.something
				word = arg.substr(4);

			Object *obj_next;
			for (Object *obj = ch->carrying; obj; obj = obj_next) {
				obj_next = obj->next_content;

				if (word.empty() // all
				 || obj->name.has_words(word))
				 	fn_helper_junk(ch, obj);
			}
		}

		return;
	}

	if (name == "purge") {
		if (arg_list.size() == 0)
			fn_helper_purge_room(ch->in_room, ch, nullptr);
		else if (arg_list[0]->type == data::Type::Character)
			fn_helper_purge_char(deref<Character *>(arg_list[0].get(), context));
		else if (arg_list[0]->type == data::Type::Object)
			fn_helper_purge_obj(deref<Object *>(arg_list[0].get(), context));

		return;
	}

	if (name == "transfer") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		Room *location = deref<Room *>(arg_list[1].get(), context);
		fn_helper_transfer(victim, location);
		return;
	}

	if (name == "invis") {
		if (deref<bool>(arg_list[0].get(), context))
			ch->act_flags += ACT_SUPERMOB;
		else
			ch->act_flags -= ACT_SUPERMOB;

		return;
	}

	if (name == "become_pet") {
		if (!ch->is_npc())
			throw String("ch is not a mob");

		Character *master = deref<Character *>(arg_list[0].get(), context);

		if (master == nullptr)
			throw String("master is null");

		// fix up old pet if any
		if (master->pet != nullptr)
			stop_follower(master->pet);

		make_pet(master, ch);
		return;
	}

	if (name == "call") {
		const String key = deref<String>(arg_list[0].get(), context);
		const String args = deref<String>(arg_list[1].get(), context);
		progs::call_trigger(ch, key, args);
		return;
	}

	if (name == "state") {
		const String key = deref<String>(arg_list[0].get(), context);
		const String value = deref<String>(arg_list[1].get(), context);

		if (key.empty() || value.empty())
			throw String("empty key or value");

		if (value == "++") {
			ch->state.increment(key);
			return;
		}

		if (value == "--") {
			ch->state.decrement(key);
			return;
		}

		ch->state.set(key, value);
		return;
	}

	if (name == "quest_assign") {
		const String quest_id = deref<String>(arg_list[0].get(), context);
		fn_helper_quest_assign(ch, quest_id);
		return;
	}

	if (name == "quest_progress") {
		const String quest_id = deref<String>(arg_list[0].get(), context);
		fn_helper_quest_progress(ch, quest_id);
		return;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Object, return Void
template <>
void
eval_delegate_void(Object *obj, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "to_char") {
		Character *ch = deref<Character *>(arg_list[0].get(), context);

		if (ch == nullptr)
			return;

		if (obj->in_room)               obj_from_room(obj);
		else if (obj->carried_by)       obj_from_char(obj);
		else if (obj->in_obj)           obj_from_obj(obj);
		else if (obj->in_locker)        obj_from_locker(obj);
		else if (obj->in_strongbox)     obj_from_strongbox(obj);

		obj_to_char(obj, ch);
		return;
	}

	if (name == "to_room") {
		Room *room = deref<Room *>(arg_list[0].get(), context);

		if (room == nullptr)
			return;

		if (obj->in_room)               obj_from_room(obj);
		else if (obj->carried_by)       obj_from_char(obj);
		else if (obj->in_obj)           obj_from_obj(obj);
		else if (obj->in_locker)        obj_from_locker(obj);
		else if (obj->in_strongbox)     obj_from_strongbox(obj);

		obj_to_room(obj, room);
		return;
	}

	if (name == "echo") {
		Room *room = obj->carried_by ? obj->carried_by->in_room : obj->in_room;
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo(buf, obj->carried_by, nullptr, obj, room);
		act(buf, obj->carried_by, obj, nullptr, TO_ROOM, POS_RESTING, false, room);
		return;
	}

	if (name == "echo_to") {
		Room *room = obj->carried_by ? obj->carried_by->in_room : obj->in_room;
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_to(buf, obj->carried_by, obj, victim, room);
		return;
	}

	if (name == "echo_other") {
		Room *room = obj->carried_by ? obj->carried_by->in_room : obj->in_room;
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_other(buf, obj->carried_by, obj, victim, room);
		return;
	}
	
	if (name == "echo_near") {
		Room *room = obj->carried_by ? obj->carried_by->in_room : obj->in_room;
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo_near(buf, obj->carried_by, obj, nullptr, room);
		return;
	}

	if (name == "from_room") {
		obj_from_room(obj);
		return;
	}

	if (name == "purge") {
		if (arg_list.size() == 0)
			fn_helper_purge_room(obj->carried_by ? obj->carried_by->in_room : obj->in_room, obj->carried_by, obj);
		else if (arg_list[0]->type == data::Type::Character)
			fn_helper_purge_char(deref<Character *>(arg_list[0].get(), context));
		else if (arg_list[0]->type == data::Type::Object)
			fn_helper_purge_obj(deref<Object *>(arg_list[0].get(), context));

		return;
	}

	if (name == "transfer") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		Room *location = deref<Room *>(arg_list[1].get(), context);
		fn_helper_transfer(victim, location);
		return;
	}

	throw Format::format("unhandled function '%s'", name);
}

// functions that access Room, return Void
template <>
void
eval_delegate_void(Room *room, const String& name, std::vector<std::unique_ptr<Symbol>>& arg_list, contexts::Context& context) {
	if (name == "echo") {
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo(buf, nullptr, nullptr, nullptr, room);
		return;
	}

	if (name == "echo_to") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_to(buf, nullptr, nullptr, victim, room);
		return;
	}

	if (name == "echo_other") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		String buf = expand(deref<String>(arg_list[1].get(), context), context);
		fn_helper_echo_other(buf, nullptr, nullptr, victim, room);
		return;
	}
	
	if (name == "echo_near") {
		String buf = expand(deref<String>(arg_list[0].get(), context), context);
		fn_helper_echo_near(buf, nullptr, nullptr, nullptr, room);
		return;
	}

	if (name == "purge") {
		if (arg_list.size() == 0)
			fn_helper_purge_room(room, nullptr, nullptr);
		else if (arg_list[0]->type == data::Type::Character)
			fn_helper_purge_char(deref<Character *>(arg_list[0].get(), context));
		else if (arg_list[0]->type == data::Type::Object)
			fn_helper_purge_obj(deref<Object *>(arg_list[0].get(), context));

		return;
	}

	if (name == "transfer") {
		Character *victim = deref<Character *>(arg_list[0].get(), context);
		Room *location = deref<Room *>(arg_list[1].get(), context);
		fn_helper_transfer(victim, location);
		return;
	}

	throw Format::format("unhandled function '%s'", name);
}

// *******************************
// Boilerplate template code below
// *******************************

template <>
Character * FunctionSymbol<Character *>::
evaluate(contexts::Context& context) {
	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_char(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

template <>
Object * FunctionSymbol<Object *>::
evaluate(contexts::Context& context) {
	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_obj(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

template <>
Room * FunctionSymbol<Room *>::
evaluate(contexts::Context& context) {
	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_room(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

template <>
String FunctionSymbol<String>::
evaluate(contexts::Context& context) {
	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_str(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

template <>
bool FunctionSymbol<bool>::
evaluate(contexts::Context& context) {
	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_bool(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

// this one isn't a template, it handles the 'Void' types that are an alias for
// 'int' because you can't have a void template.  The int template will delegate here
void
evaluate_void(FunctionSymbol<int>& sym, contexts::Context& context) {
	const String& name = fn_table[sym.fn_index].name;

try {
	if (sym.parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (sym.parent->type) {
	case data::Type::World: {
		return eval_delegate_world_void(name, sym.arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(sym.parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

		return eval_delegate_void(ch, name, sym.arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(sym.parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

		return eval_delegate_void(obj, name, sym.arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(sym.parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

		return eval_delegate_void(room, name, sym.arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(sym.parent.get(), context);
		return eval_delegate_void(str, name, sym.arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(sym.parent.get(), context);
		return eval_delegate_void(val, name, sym.arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(sym.parent.get(), context);
		return eval_delegate_void(num, name, sym.arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(sym.type), name, e);
}
}

template <>
int FunctionSymbol<int>::
evaluate(contexts::Context& context) {
	// delegate handling of aliased 'Void' type
	if (type == data::Type::Void) {
		evaluate_void(*this, context);
		return 1; // boolean true, I guess?
	}

	const String& name = fn_table[fn_index].name;

try {
	if (parent == nullptr)
		throw Format::format("null parent symbol", name);

	switch (parent->type) {
	case data::Type::World: {
		return eval_delegate_world_int(name, arg_list, context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(ch, name, arg_list, context);
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(obj, name, arg_list, context);
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		return eval_delegate(room, name, arg_list, context);
	}
	case data::Type::String: {
		const String str = deref<String>(parent.get(), context);
		return eval_delegate(str, name, arg_list, context);
	}
	case data::Type::Boolean: {
		bool val = deref<bool>(parent.get(), context);
		return eval_delegate(val, name, arg_list, context);
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);
		return eval_delegate(num, name, arg_list, context);
	}
	case data::Type::Void:
		throw String("member function of Void type called");
	}
} catch(String e) {
	throw Format::format("progs::FunctionSymbol<%s>::evaluate::%s(): %s", type_to_string(type), name, e);
}
}

} // namespace symbols
} // namespace progs

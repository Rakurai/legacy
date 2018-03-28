#include "progs/symbols/declare.hh"
#include "progs/symbols/FunctionSymbol.hh"
#include "progs/symbols/deref.hh"
#include "progs/contexts/Context.hh"
#include "Character.hh"
#include "Object.hh"
#include "Game.hh"
#include "World.hh"
#include "Room.hh"
#include "Logging.hh"
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


namespace progs {
namespace symbols {

#define dt data::Type

const std::vector<fn_type> fn_table = {
	// name          return type    parent type    arg types

	// conversions, so we don't have to overload everything
	{ "cv_to_room",  dt::Room,      dt::String,    {} }, // lookup a room by location (map/vnum)
	{ "cv_to_room",  dt::Room,      dt::Integer,   {} }, // lookup a room by vnum
	{ "cv_to_room",  dt::Room,      dt::Character, {} }, // lookup a room by character

	{ "cv_to_char",  dt::Character, dt::String,    {} }, // find a character in the $room

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
	{ "do",          dt::Void,      dt::Character, { dt::String } },
	{ "goto",        dt::Void,      dt::Character, { dt::Room } },
	{ "echo",        dt::Void,      dt::Character, { dt::String } },
	{ "echo_near",   dt::Void,      dt::Character, { dt::String } },
	{ "cast",        dt::Void,      dt::Character, { dt::String } },
	{ "at",          dt::Void,      dt::Character, { dt::Room, dt::Void } },
	{ "kill",        dt::Void,      dt::Character, { dt::Character } },
	{ "junk",        dt::Void,      dt::Character, { dt::String } }, // first, could be all.something
	{ "junk",        dt::Void,      dt::Character, { dt::Object } }, // overloaded for specific obj

	// object accessors
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
	{ "from_room",   dt::Void,      dt::Object,    {} },
	{ "to_char",     dt::Void,      dt::Object,    { dt::Character } },
	{ "echo",        dt::Void,      dt::Object,    { dt::String } },
	{ "echo_near",   dt::Void,      dt::Object,    { dt::String } },

	// room accessors
	{ "name",        dt::String,    dt::Room,      {} },
	{ "vnum",        dt::Integer,   dt::Room,      {} },
	{ "get_char",    dt::Character, dt::Room,      { dt::String } }, // lookup by name
	{ "get_obj",     dt::Object,    dt::Room,      { dt::String } }, // lookup by name

	// room actions
	{ "load_mob",    dt::Character, dt::Room,      { dt::Integer } },
	{ "load_obj",    dt::Object,    dt::Room,      { dt::Integer } },
	{ "echo",        dt::Void,      dt::Room,      { dt::String } },
	{ "echo_near",   dt::Void,      dt::Room,      { dt::String } },
};

#undef dt

template <>
Character * FunctionSymbol<Character *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::World: {
		if (name == "get_char") {
			String str = deref<const String>(arg_list[0].get(), context);
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
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "master") return ch->master;
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "get_char") {
			return fn_helper_get_char(deref<const String>(arg_list[0].get(), context), context, room);
		}

		if (name == "load_mob") {
			int vnum = deref<int>(arg_list[0].get(), context);
			MobilePrototype *proto = Game::world().get_mob_prototype(vnum);

			if (proto == nullptr) {
				Logging::bugf("room.load_mob() - bad vnum %d", vnum);
				return nullptr;
			}

			Character *mob = create_mobile(proto);

			if (mob)
				char_to_room(mob, room);

			return mob;
		}
	}
	case data::Type::String: {
		const String str = deref<const String>(parent.get(), context);

		if (name == "cv_to_char") {
			Room *room;
			context.get_var("room", &room);
			return fn_helper_get_char(str, context, room);
		}
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'Character *'", e);
}
}

template <>
Object * FunctionSymbol<Object *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::World: {
		if (name == "get_obj") {
			String str = deref<const String>(arg_list[0].get(), context);
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
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "load_obj") {
			int vnum = deref<int>(arg_list[0].get(), context);
			ObjectPrototype *proto = Game::world().get_obj_prototype(vnum);

			if (proto == nullptr) {
				Logging::bugf("character.load_obj() - bad vnum %d", vnum);
				return nullptr;
			}

			Object *obj = create_object(proto, 0);

			if (obj) {
				if (CAN_WEAR(obj, ITEM_TAKE))
					obj_to_char(obj, ch);
				else
					obj_to_room(obj, ch->in_room);
			}

			return obj;
		}
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "get_obj") {
			String str = deref<const String>(arg_list[0].get(), context);
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

			if (proto == nullptr) {
				Logging::bugf("room.load_obj() - bad vnum %d", vnum);
				return nullptr;
			}

			Object *obj = create_object(proto, 0);

			if (obj)
				obj_to_room(obj, room);

			return obj;
		}
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'Object *'", e);
}
}

template <>
Room * FunctionSymbol<Room *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::World: {
		if (name == "get_room") return deref<Room *>(arg_list[0].get(), context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "cv_to_room") return ch->in_room;
	}
	case data::Type::String: {
		const String str = deref<const String>(parent.get(), context);

		if (name == "cv_to_room") {
			Location location(str);
			Room *room;

			if (!location.is_valid()
			 || (room = Game::world().get_room(location)) == nullptr)
				throw Format::format("get_room(): bad location '%s'", str);

			return room;
		}
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);

		if (name == "cv_to_room") {
			Location location((Vnum(num)));
			Room *room;

			if (!location.is_valid()
			 || (room = Game::world().get_room(location)) == nullptr)
				throw Format::format("get_room(): bad location '%d'", num);

			return room;
		}
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'Room *'", e);
}
}

template <>
const String FunctionSymbol<const String>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::World: { // global function
		if (name == "time")    return Format::format("%d", Game::world().time.hour);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		bool can_see = context.can_see(ch);

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
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		bool can_see = context.can_see(obj);

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
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		bool can_see = context.can_see(room);

		if (name == "name") {
			if (!can_see)     return "somewhere";
			                  return room->name();
		}
	}
	case data::Type::Integer: {
		int num = deref<int>(parent.get(), context);

		if (name == "cv_to_str") { std::cout << "converting\n"; return Format::format("%d", num); }
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'const String'", e);
}
}

template <>
bool FunctionSymbol<bool>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::World: { // global function
		if (name == "rand")    return number_percent() <= deref<int>(arg_list[0].get(), context);
	}
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

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
		if (name == "is_carrying") return get_obj_carry(ch, deref<const String>(arg_list[0].get(), context)) != nullptr;
		if (name == "is_wearing")  return get_obj_wear(ch, deref<const String>(arg_list[0].get(), context)) != nullptr;
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'bool'", e);
}
}

// this one isn't a template, it handles the 'Void' types that are an alias for
// 'int' because you can't have a void template.  The int template will delegate here
void evaluate_void(FunctionSymbol<int>& sym, contexts::Context& context) {
try {
	const String& name = fn_table[sym.fn_index].name;

	if (sym.parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (sym.parent->type) {
	case data::Type::Character: {
		Character *ch = deref<Character *>(sym.parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

		if (name == "do") {
			String argument = deref<const String>(sym.arg_list[0].get(), context);
			interpret(ch, argument);
			return;
		}
		
		if (name == "to_room") {
			Room *room = deref<Room *>(sym.arg_list[0].get(), context);

			if (room == nullptr)
				throw Format::format("char:to_room: room is null");

			if (ch->fighting != nullptr)
				stop_fighting(ch, true);

			char_from_room(ch);
			char_to_room(ch, room);
			return;
		}
		
		if (name == "echo") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			fn_helper_echo(buf, nullptr, ch, nullptr);
			return;
		}
		
		if (name == "echo_near") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			fn_helper_echo_near(buf, ch->in_room);
			return;
		}
		
		if (name == "cast") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			do_mpcast(ch, buf);
			return;
		}
		
		if (name == "at") {
			Room *room = deref<Room *>(sym.arg_list[0].get(), context);

			if (room == nullptr)
				throw Format::format("char:at: room is null");

			Room *old_room = ch->in_room;

			char_from_room(ch);
			char_to_room(ch, room);

			deref<int>(sym.arg_list[0].get(), context); // execute

			if (!ch->is_garbage()) {
				char_from_room(ch);
				char_to_room(ch, old_room);
			}

			return;
		}

		if (name == "kill") {
			Character *victim = deref<Character *>(sym.arg_list[0].get(), context);

			if (victim == nullptr)
				throw Format::format("char::kill: victim is null");

			if (ch->in_room != victim->in_room)
				throw Format::format("char::kill: victim not in same room");

			if (affect::exists_on_char(ch, affect::type::charm_person)
			 && ch->master == victim)
				throw Format::format("char::kill: charmed mob attacking master");

			if (ch->fighting)
				throw Format::format("char::kill: already fighting");

			multi_hit(ch, victim, skill::type::unknown);
			return;
		}

		if (name == "junk") {
			// overloaded, could be Object or String
			if (sym.arg_list[0]->type == data::Type::Object) {
				fn_helper_junk(ch, deref<Object *>(sym.arg_list[0].get(), context));
				return;
			}

			const String args = deref<const String>(sym.arg_list[0].get(), context);
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
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(sym.parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

		if (name == "echo") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			fn_helper_echo(buf, nullptr, nullptr, obj);
			return;
		}
		
		if (name == "echo_near") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			Room *room = obj->carried_by ? obj->carried_by->in_room : obj->in_room;

			if (room)
				fn_helper_echo_near(buf, room);

			return;
		}

		if (name == "from_room") {
			obj_from_room(obj);
			return;
		}

		if (name == "to_char") {
			obj_to_char(obj, deref<Character *>(sym.arg_list[0].get(), context));
			return;
		}
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(sym.parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(sym.parent->type));

//		bool can_see = context.can_see(room);

		if (name == "echo") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			fn_helper_echo(buf, room, nullptr, nullptr);
		}
		
		if (name == "echo_near") {
			String buf = context.expand_vars(deref<const String>(sym.arg_list[0].get(), context));
			fn_helper_echo_near(buf, room);
		}
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(sym.parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'void'", e);
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

try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	switch (parent->type) {
	case data::Type::Character: {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "position")   return ch->position;
		if (name == "level")      return ch->level;
		if (name == "guild")      return ch->guild;
		if (name == "gold")       return ch->gold;
		if (name == "vnum")       return ch->is_npc() ? ch->pIndexData->vnum.value() : 0;
		if (name == "sex")        return GET_ATTR_SEX(ch);
		if (name == "hitprcnt")   return ch->hit / GET_MAX_HIT(ch);
		if (name == "state") {
			String key = deref<const String>(arg_list[0].get(), context);

			if (!key.empty()) {
				const auto entry = ch->mpstate.find(key);

				if (entry != ch->mpstate.cend())
					return entry->second;
			}

			return 0;
		}
	}
	case data::Type::Object: {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "type")      return obj->item_type;
		if (name == "value0")    return obj->value[0];
		if (name == "value1")    return obj->value[1];
		if (name == "value2")    return obj->value[2];
		if (name == "value3")    return obj->value[3];
		if (name == "value4")    return obj->value[4];
		if (name == "vnum")      return obj->pIndexData->vnum.value();
	}
	case data::Type::Room: {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

//		bool can_see = context.can_see(room);

		if (name == "vnum")      return room->prototype.vnum.value();
	}
	default: break;
	}

	throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'int'", e);
}
}

} // namespace symbols
} // namespace progs

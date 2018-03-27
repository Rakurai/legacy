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


namespace progs {
namespace symbols {

#define dt data::Type

const std::vector<fn_type> fn_table = {
	// name          return type    parent type    arg types

	// world accessors
	{ "time",        dt::String,    dt::World,     {} },
	{ "rand",        dt::Boolean,   dt::World,     { dt::Integer } },
	{ "get_room",    dt::Room,      dt::World,     { dt::Integer } },
	{ "get_room",    dt::Room,      dt::World,     { dt::String } },

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
	{ "to_room",     dt::Void,      dt::Character, { dt::Room } },
	{ "to_room",     dt::Void,      dt::Character, { dt::Integer } },
	{ "to_room",     dt::Void,      dt::Character, { dt::String } },
	{ "echo",        dt::Void,      dt::Character, { dt::String } },
	{ "cast",        dt::Void,      dt::Character, { dt::String } },
	{ "at",          dt::Void,      dt::Character, { dt::Room, dt::Void } },
	{ "at",          dt::Void,      dt::Character, { dt::String, dt::Void } },
	{ "at",          dt::Void,      dt::Character, { dt::Integer, dt::Void } },

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

	// room accessors
	{ "name",        dt::String,    dt::Room,      {} },
	{ "vnum",        dt::Integer,   dt::Room,      {} },

	// room actions
	{ "load_mob",    dt::Character, dt::Room,      { dt::Integer } },
	{ "load_obj",    dt::Object,    dt::Room,      { dt::Integer } },
	{ "echo",        dt::Void,      dt::Room,      { dt::String } },
};

#undef dt

template <>
Character * FunctionSymbol<Character *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	if (parent->type == data::Type::World) {

	}

	if (parent->type == data::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "master") return ch->master;

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Room) {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
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

	if (parent->type == data::Type::Character) {
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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Room) {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
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

	if (parent->type == data::Type::World) { // global function
		if (name == "get_room") {
			Location location;

			if (arg_list[0]->type == data::Type::String)
				location = Location(deref<const String>(arg_list[0].get(), context));
			else if (arg_list[0]->type == data::Type::Integer)
				location = Location(deref<int>(arg_list[0].get(), context));

			if (!location.is_valid())
				throw Format::format("get_room(): bad location '%d'", location.to_string());

			Room *room = Game::world().get_room(location);

			if (room == nullptr)
				throw Format::format("get_room(): bad location '%d'", location.to_string());

			return room;
		}

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
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

	if (parent->type == data::Type::World) { // global function

		if (name == "time")    return Format::format("%d", Game::world().time.hour);

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Character) {
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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Object) {
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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Room) {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		bool can_see = context.can_see(room);

		if (name == "name") {
			if (!can_see)     return "somewhere";
			                  return room->name();
		}

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
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

	if (parent->type == data::Type::World) { // global function

		if (name == "rand")    return number_percent() <= deref<int>(arg_list[0].get(), context);

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	if (parent->type == data::Type::Character) {
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

		throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'bool'", e);
}
}

template <>
int FunctionSymbol<int>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr)
		throw Format::format("function '%s' has null parent", name);

	if (parent->type == data::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		     if (name == "position")   return ch->position;
		else if (name == "level")      return ch->level;
		else if (name == "guild")      return ch->guild;
		else if (name == "gold")       return ch->gold;
		else if (name == "vnum")       return ch->is_npc() ? ch->pIndexData->vnum.value() : 0;
		else if (name == "sex")        return GET_ATTR_SEX(ch);
		else if (name == "hitprcnt")   return ch->hit / GET_MAX_HIT(ch);
		else if (name == "state") {
			String key = deref<const String>(arg_list[0].get(), context);

			if (!key.empty()) {
				const auto entry = ch->mpstate.find(key);

				if (entry != ch->mpstate.cend())
					return entry->second;
			}
		}
		else if (name == "do") {
			String argument = deref<const String>(arg_list[0].get(), context);
			interpret(ch, argument);
		}
		else if (name == "to_room") {
			Room *room = nullptr;

			if (arg_list[0]->type == data::Type::String)
				room = Game::world().get_room(Location(deref<const String>(arg_list[0].get(), context)));
			else if (arg_list[0]->type == data::Type::Integer)
				room = Game::world().get_room(Location(deref<int>(arg_list[0].get(), context)));
			else if (arg_list[0]->type == data::Type::Room)
				room = deref<Room *>(arg_list[0].get(), context);

			if (room == nullptr)
				throw Format::format("char:to_room: room is null");

			if (ch->fighting != nullptr)
				stop_fighting(ch, true);

			char_from_room(ch);
			char_to_room(ch, room);
		}
		else if (name == "echo") {
			// uses act() to distribute the message, but act variables don't match up with prog
			// variables, so we need to do variable replacement in the string and pass a straight
			// string to act.  maybe someday we can rework this and let act do the work.
			String buf = context.expand_vars(deref<const String>(arg_list[0].get(), context));
			act(buf, ch, nullptr, nullptr, TO_ROOM);
		}
		else if (name == "cast") {
			String buf = context.expand_vars(deref<const String>(arg_list[0].get(), context));
			do_mpcast(ch, buf);
		}
		else if (name == "at") {
			Room *room = nullptr;

			if (arg_list[0]->type == data::Type::String)
				room = Game::world().get_room(Location(deref<const String>(arg_list[0].get(), context)));
			else if (arg_list[0]->type == data::Type::Integer)
				room = Game::world().get_room(Location(deref<int>(arg_list[0].get(), context)));
			else if (arg_list[0]->type == data::Type::Room)
				room = deref<Room *>(arg_list[0].get(), context);

			if (room == nullptr)
				throw Format::format("char:at: room is null");

			Room *old_room = ch->in_room;

			char_from_room(ch);
			char_to_room(ch, room);

			deref<int>(arg_list[0].get(), context); // execute

			if (!ch->is_garbage()) {
				char_from_room(ch);
				char_to_room(ch, old_room);
			}
		}
		else
			throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);

		return 0;
	}

	if (parent->type == data::Type::Object) {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

		if (name == "echo") {
			// uses act() to distribute the message, but act variables don't match up with prog
			// variables, so we need to do variable replacement in the string and pass a straight
			// string to act.  maybe someday we can rework this and let act do the work.
			String buf = context.expand_vars(deref<const String>(arg_list[0].get(), context));
			act(buf, nullptr, obj, nullptr, TO_ROOM);
		}
		else if (name == "from_room") obj_from_room(obj);
		else if (name == "to_char")   obj_to_char(obj, deref<Character *>(arg_list[0].get(), context));
		else if (name == "type")      return obj->item_type;
		else if (name == "value0")    return obj->value[0];
		else if (name == "value1")    return obj->value[1];
		else if (name == "value2")    return obj->value[2];
		else if (name == "value3")    return obj->value[3];
		else if (name == "value4")    return obj->value[4];
		else if (name == "vnum")      return obj->pIndexData->vnum.value();
		else
			throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);

		return 0;
	}

	if (parent->type == data::Type::Room) {
		Room *room = deref<Room *>(parent.get(), context);

		if (room == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", type_to_string(parent->type));

//		bool can_see = context.can_see(room);

		if (name == "vnum")      return room->prototype.vnum.value();
		if (name == "echo") {
			// uses act() to distribute the message, but act variables don't match up with prog
			// variables, so we need to do variable replacement in the string and pass a straight
			// string to act.  maybe someday we can rework this and let act do the work.

			// someday we'll pass a room to act, for now just send to everyone
			String buf = context.expand_vars(deref<const String>(arg_list[0].get(), context));

			for (Character *ch = room->people; ch; ch = ch->next_in_room)
				stc(buf + "\n", ch);
		}
		else
			throw Format::format("unhandled %s function '%s'", type_to_string(parent->type), name);

		return 0;
	}

	throw Format::format("unhandled parent class '%s'", type_to_string(parent->type));
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'int'", e);
}
}

} // namespace symbols
} // namespace progs

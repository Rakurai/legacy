#include "progs/symbols/deref.hh"

#include "progs/symbols/declare.hh"
#include "progs/symbols/FunctionSymbol.hh"
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
#include "find.hh"


namespace progs {
namespace symbols {

const std::vector<fn_type> fn_table = {
	// name          return type              context                  arg types
	// global context
	{ "mudtime",     Symbol::Type::String,    Symbol::Type::global,    {} },
	{ "rand",        Symbol::Type::Boolean,   Symbol::Type::global,    { Symbol::Type::Integer } },

	// character accessors
	{ "name",        Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "title",       Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "he_she",      Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "him_her",     Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "his_her",     Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "is_pc",       Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_npc",      Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_good",     Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_evil",     Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_neutral",  Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_immort",   Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_fighting", Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_killer",   Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_thief",    Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "is_charmed",  Symbol::Type::Boolean,   Symbol::Type::Character, {} },
	{ "in_room",     Symbol::Type::String,    Symbol::Type::Character, {} },
	{ "position",    Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "level",       Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "hitprcnt",    Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "sex",         Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "guild",       Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "gold",        Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "vnum",        Symbol::Type::Integer,   Symbol::Type::Character, {} },
	{ "state",       Symbol::Type::Integer,   Symbol::Type::Character, { Symbol::Type::String } },
	{ "is_carrying", Symbol::Type::Boolean,   Symbol::Type::Character, { Symbol::Type::String } },
	{ "is_wearing",  Symbol::Type::Boolean,   Symbol::Type::Character, { Symbol::Type::String } },
	{ "master",      Symbol::Type::Character, Symbol::Type::Character, {} },

	// object accessors
	{ "name",        Symbol::Type::String,    Symbol::Type::Object,    {} },
	{ "sdesc",       Symbol::Type::String,    Symbol::Type::Object,    {} },
	{ "ind_art",     Symbol::Type::String,    Symbol::Type::Object,    {} },
	{ "echo",        Symbol::Type::Void,      Symbol::Type::Object,    { Symbol::Type::String } },
	{ "from_room",   Symbol::Type::Void,      Symbol::Type::Object,    {} },
	{ "to_char",     Symbol::Type::Void,      Symbol::Type::Object,    { Symbol::Type::Character } },
	{ "type",        Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "value0",      Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "value1",      Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "value2",      Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "value3",      Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "value4",      Symbol::Type::Integer,   Symbol::Type::Object,    {} },
	{ "vnum",        Symbol::Type::Integer,   Symbol::Type::Object,    {} },


	// room accessors

};

template <>
Character * FunctionSymbol<Character *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr) { // global function
		throw Format::format("unhandled global function '%s'", name);
	}

	if (parent->type == Symbol::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

		if (name == "master") return ch->master;

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	throw Format::format("unhandled parent class '%s'", parent->type_to_string());
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'Character *'", e);
}
}

template <>
Object * FunctionSymbol<Object *>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr) { // global function
		throw Format::format("unhandled global function '%s'", name);
	}

	if (parent->type == Symbol::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	throw Format::format("unhandled parent class '%s'", parent->type_to_string());
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'Object *'", e);
}
}

template <>
const String FunctionSymbol<const String>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr) { // global function

		if (name == "mudtime")    return Format::format("%d", Game::world().time.hour);

		throw Format::format("unhandled global function '%s'", name);
	}

	if (parent->type == Symbol::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

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

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	if (parent->type == Symbol::Type::Object) {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

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

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	throw Format::format("unhandled parent class '%s'", parent->type_to_string());
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'const String'", e);
}
}

template <>
bool FunctionSymbol<bool>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr) { // global function

		if (name == "rand")       return number_percent() <= deref<int>(arg_list[0].get(), context);

		throw Format::format("unhandled global function '%s'", name);
	}

	if (parent->type == Symbol::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

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

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	throw Format::format("unhandled parent class '%s'", parent->type_to_string());
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'bool'", e);
}
}

template <>
int FunctionSymbol<int>::
evaluate(contexts::Context& context) {
try {
	const String& name = fn_table[fn_index].name;

	if (parent == nullptr) { // global function
		throw Format::format("unhandled global function '%s'", name);
	}

	if (parent->type == Symbol::Type::Character) {
		Character *ch = deref<Character *>(parent.get(), context);

		if (ch == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

		if (name == "position")   return ch->position;
		if (name == "level")      return ch->level;
		if (name == "guild")      return ch->guild;
		if (name == "gold")       return ch->gold;
		if (name == "vnum")       return ch->is_npc() ? ch->pIndexData->vnum.value() : 0;
		if (name == "sex")        return GET_ATTR_SEX(ch);
		if (name == "hitprcnt")   return ch->hit / GET_MAX_HIT(ch);
		if (name == "state") {
			String key = deref<const String>(arg_list[0].get(), context);
			const auto entry = ch->mpstate.find(key);

			if (entry != ch->mpstate.cend())
				return entry->second;

			return 0;
		}

		throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);
	}

	if (parent->type == Symbol::Type::Object) {
		Object *obj = deref<Object *>(parent.get(), context);

		if (obj == nullptr)
			throw Format::format("dereferenced %s parent pointer is null", parent->type_to_string());

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
			throw Format::format("unhandled %s function '%s'", parent->type_to_string(), name);

		return 0;
	}

	throw Format::format("unhandled parent class '%s'", parent->type_to_string());
} catch(String e) {
	throw Format::format("progs::FunctionSymbol::evaluate: %s, return type 'int'", e);
}
}

} // namespace symbols
} // namespace progs

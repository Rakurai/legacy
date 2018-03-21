#include "progs/Context.hh"
#include "merc.hh"
#include "Character.hh"
#include "Object.hh"
#include "Room.hh"
#include "random.hh"
#include "Game.hh"
#include "World.hh"
#include "find.hh"
#include "affect/Affect.hh"
#include "MobilePrototype.hh"
#include "ObjectPrototype.hh"

namespace progs {

const String Context::
access_member(const Character *ch, const String& member_name, bool can_see) const {
	static const char *he_she        [] = { "it",  "he",  "she" };
	static const char *him_her       [] = { "it",  "him", "her" };
	static const char *his_her       [] = { "its", "his", "her" };

	if (member_name == "name") {
		if (!can_see)     return "someone";
		if (ch->is_npc()) return ch->name.lsplit().capitalize();
		                  return ch->name;
	}

	if (member_name == "title") {
		if (!can_see)     return "someone";
		if (ch->is_npc()) return ch->short_descr;
		                  return ch->name + " " + ch->pcdata->title;
	}

	if (member_name == "he_she") {
		if (!can_see)     return "someone";
		                  return he_she[GET_ATTR_SEX(actor)];
	}

	if (member_name == "him_her") {
		if (!can_see)     return "someone";
		                  return him_her[GET_ATTR_SEX(actor)];
	}

	if (member_name == "his_her") {
		if (!can_see)     return "someone's";
		                  return his_her[GET_ATTR_SEX(actor)];
	}

	throw Format::format("progs::access_member: unknown Character member name '%s'", member_name);
}

const String Context::
access_member(const Object *obj, const String& member_name, bool can_see) const {
	if (member_name == "name") {
		if (!can_see)     return "something";
		                  return obj->name.lsplit();
	}

	if (member_name == "sdesc") {
		if (!can_see)     return "something";
		                  return obj->short_descr;
	}

	if (member_name == "ind_art") {
		char c = tolower(obj->name[0]);

		if (can_see && (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'))
			return "an";

		return "a";
	}

	throw Format::format("progs::access_member: unknown Object member name '%s'", member_name);
}

const String Context::
compute_function(const String& fn, const std::vector<std::unique_ptr<Symbol>>& arg_list) const {
	// functions with no args
	if (!strcmp(fn, "mudtime"))
		return Format::format("%d", Game::world().time.hour);

	// functions with 1 arg

	if (!strcmp(fn, "rand"))
		return (number_percent() <= atoi(arg_list[0]->to_string(*this))) ? "1" : "0";

	if (!strcmp(fn, "ispc")) {
		return !get_char_target(arg_list[0])->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isnpc")) {
		return get_char_target(arg_list[0])->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isgood")) {
		return IS_GOOD(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "isevil")) {
		return IS_EVIL(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "isneutral")) {
		return IS_NEUTRAL(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "isfight")) {
		return get_char_target(arg_list[0])->fighting ? "1" : "0";
	}

	if (!strcmp(fn, "isimmort")) {
		return IS_IMMORTAL(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "iskiller")) {
		return IS_KILLER(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "isthief")) {
		return IS_THIEF(get_char_target(arg_list[0])) ? "1" : "0";
	}

	if (!strcmp(fn, "ischarmed")) {
		return affect::exists_on_char(get_char_target(arg_list[0]), affect::type::charm_person) ? "1" : "0";
	}

	if (!strcmp(fn, "isfollow")) {
		Character *target = get_char_target(arg_list[0]);
		return (target->master && target->master->in_room == target->in_room) ? "1" : "0";
	}

	if (!strcmp(fn, "hitprcnt")) {
		Character *target = get_char_target(arg_list[0]);
		return Format::format("%d", target->hit / GET_MAX_HIT(target));
	}

	if (!strcmp(fn, "inroom")) {
		return get_char_target(arg_list[0])->in_room->location.to_string();
	}

	if (!strcmp(fn, "sex")) {
		return Format::format("%d", GET_ATTR_SEX(get_char_target(arg_list[0])));
	}

	if (!strcmp(fn, "position")) {
		return Format::format("%d", get_char_target(arg_list[0])->position);
	}

	if (!strcmp(fn, "level")) {
		return Format::format("%d", get_char_target(arg_list[0])->level);
	}

	if (!strcmp(fn, "class")) {
		return Format::format("%d", get_char_target(arg_list[0])->guild);
	}

	if (!strcmp(fn, "goldamt")) {
		return Format::format("%d", get_char_target(arg_list[0])->gold);
	}

	if (!strcmp(fn, "objtype")) {
		return Format::format("%d", get_obj_target(arg_list[0])->item_type);
	}

	if (!strcmp(fn, "objval0")) {
		return Format::format("%d", get_obj_target(arg_list[0])->value[0]);
	}

	if (!strcmp(fn, "objval1")) {
		return Format::format("%d", get_obj_target(arg_list[0])->value[1]);
	}

	if (!strcmp(fn, "objval2")) {
		return Format::format("%d", get_obj_target(arg_list[0])->value[2]);
	}

	if (!strcmp(fn, "objval3")) {
		return Format::format("%d", get_obj_target(arg_list[0])->value[3]);
	}

	if (!strcmp(fn, "objval4")) {
		return Format::format("%d", get_obj_target(arg_list[0])->value[4]);
	}

	if (!strcmp(fn, "number")) {
		// try it as a character
		try {
			return Format::format("%d", get_char_target(arg_list[0])->pIndexData->vnum);
		}
		// try it as an obj
		catch (String e) {
			return Format::format("%d", get_obj_target(arg_list[0])->pIndexData->vnum);
		}
	}

	if (!strcmp(fn, "name")) {
		// try it as a character
		try {
			return get_char_target(arg_list[0])->name;
		}
		// try it as an obj
		catch (String e) {
			return get_obj_target(arg_list[0])->name;
		}
	}

	// functions with 2 args

	if (!strcmp(fn, "get_state")) {
		if (arg_list[0]->empty() || arg_list[1]->empty())
			throw Format::format("progs::Context::compute_function: not enough args for '%s'", fn);

		Character *target = get_char_target(arg_list[0]);
		const auto entry = target->mpstate.find(arg_list[1]);

		int state = 0;

		if (entry != target->mpstate.cend())
			state = entry->second;

		return Format::format("%d", state);
	}

	if (!strcmp(fn, "iscarrying")) {
		Character *target = get_char_target(arg_list[0]);
		return (get_obj_carry(target, arg_list[1]) == nullptr) ? "0" : "1";
	}

	if (!strcmp(fn, "iswearing")) {
		Character *target = get_char_target(arg_list[0]);
		return (get_obj_wear(target, arg_list[1]) == nullptr) ? "0" : "1";
	}

	throw Format::format("progs::compute_function: unknown function name '%s'", fn);
}

} // namespace progs

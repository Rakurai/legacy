#include "progs/MobProgContext.hh"
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

bool MOBtrigger;

const Vnum MobProgContext::
vnum() const {
	return mob->pIndexData->vnum;
}

bool MobProgContext::
self_is_garbage() const {
	return mob->is_garbage();
}

MobProgContext::
MobProgContext(Character *mob, Character *actor, Object *obj, void *vo) {
	this->self = mob;
	this->actor = actor;
	this->obj = obj;
	this->vo = vo;

	this->mob = mob;

	int count = 0;
	/* get a random visable mortal player who is in the room with the mob */
	for (Character *vch = mob->in_room->people; vch; vch = vch->next_in_room) {
		if (!vch->is_npc()
		    &&  !IS_IMMORTAL(vch)
		    &&  can_see_char(mob, vch)) {
			if (number_range(0, count) == 0)
				rndm = vch;

			count++;
		}
	}
}

// tired of all this duplicate code
Character * MobProgContext::
get_char_target(const String& var) const {
	if (var.length() == 2)
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'i': return (Character *)self;
			case 'b': return ((Character *)self)->master;
			case 'n': return actor;
			case 't': return (Character *)vo;
			case 'r': return rndm;
		}

	throw Format::format("progs::Context::get_char_target: bad target '%s'", var);
}

Object * MobProgContext::
get_obj_target(const String& var) const {
	if (var.length() == 2)
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'o': return obj;
			case 'p': return (Object *)vo;
		}

	throw Format::format("progs::Context::get_obj_target: bad target '%s'", var);
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
String MobProgContext::
translate(char ch) const {
	static char *he_she        [] = { "it",  "he",  "she" };
	static char *him_her       [] = { "it",  "him", "her" };
	static char *his_her       [] = { "its", "his", "her" };
	Character   *vict             = (Character *) vo;
	Object    *v_obj            = (Object *) vo;
	String t;

	switch (ch) {
	case 'i':
		t = mob->name.lsplit();
		break;

	case 'I':
		t = mob->short_descr;
		break;

	case 'n':
		if (actor) {
			if (can_see_char(mob, actor))
				t = actor->name;

			if (!actor->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'N':
		if (actor) {
			if (can_see_char(mob, actor)) {
				if (actor->is_npc())
					t = actor->short_descr;
				else {
					t = actor->name;
					t += " ";
					t += actor->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'b':
		if (mob->master) {
			if (can_see_char(mob, mob->master))
				t = mob->master->name;

			if (!mob->master->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'B':
		if (mob->master) {
			if (can_see_char(mob, mob->master)) {
				if (mob->master->is_npc())
					t = mob->master->short_descr;
				else {
					t = mob->master->name;
					t += " ";
					t += mob->master->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 't':
		if (vict) {
			if (can_see_char(mob, vict))
				t = vict->name;

			if (!vict->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'T':
		if (vict) {
			if (can_see_char(mob, vict)) {
				if (vict->is_npc())
					t = vict->short_descr;
				else {
					t = vict->name;
					t += " ";
					t += vict->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'r':
		if (rndm) {
			if (can_see_char(mob, rndm))
				t = rndm->name.lsplit();
			else
				t = "someone";

			if (!rndm->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'R':
		if (rndm) {
			if (can_see_char(mob, rndm)) {
				if (rndm->is_npc())
					t = rndm->short_descr;
				else {
					t = rndm->name;
					t += " ";
					t += rndm->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'e':
		if (actor)
			t = can_see_char(mob, actor) ? he_she[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 'm':
		if (actor)
			t = can_see_char(mob, actor) ? him_her[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 's':
		if (actor)
			t = can_see_char(mob, actor) ? his_her[GET_ATTR_SEX(actor)] : "someone's";

		break;

	case 'E':
		if (vict)
			t = can_see_char(mob, vict) ? he_she[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'M':
		if (vict)
			t = can_see_char(mob, vict) ? him_her[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'S':
		if (vict)
			t = can_see_char(mob, vict) ? his_her[GET_ATTR_SEX(vict)] : "someone's";

		break;

	case 'f':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? he_she[GET_ATTR_SEX(mob->master)] : "someone";

		break;

	case 'g':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? him_her[GET_ATTR_SEX(mob->master)] : "someone";

		break;

	case 'h':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? his_her[GET_ATTR_SEX(mob->master)] : "someone's";

		break;

	case 'j':
		t = he_she[GET_ATTR_SEX(mob)];
		break;

	case 'k':
		t = him_her[GET_ATTR_SEX(mob)];
		break;

	case 'l':
		t = his_her[GET_ATTR_SEX(mob)];
		break;

	case 'J':
		if (rndm)
			t = can_see_char(mob, rndm) ? he_she[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'K':
		if (rndm)
			t = can_see_char(mob, rndm) ? him_her[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'L':
		if (rndm)
			t = can_see_char(mob, rndm) ? his_her[GET_ATTR_SEX(rndm)] : "someone's";

		break;

	case 'o':
		if (obj)
			t = can_see_obj(mob, obj) ? String(obj->name).lsplit() : "something";

		break;

	case 'O':
		if (obj)
			t = can_see_obj(mob, obj) ? obj->short_descr : "something";

		break;

	case 'p':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? String(v_obj->name).lsplit() : "something";

		break;

	case 'P':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? v_obj->short_descr : "something";

		break;

	case 'a':
		if (obj)
			switch (tolower(obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case 'A':
		if (v_obj)
			switch (tolower(v_obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case '$':
		t = "$";
		break;

	default:
		throw Format::format("progs::Context::translate: bad variable '$%c'", ch);
		break;
	}

	return t;
}

const String MobProgContext::
dereference_variable(const String& var, String member_name) const {
	if (member_name.empty())
		member_name = "name";

	// try it as a character
	try {
		Character *target = get_char_target(var);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", var);

		if (member_name == "name") return target->name;

		throw Format::format("progs::dereference_variable: unknown variable member '%s'", member_name);
	}
	catch (String e) {
		throw;
	}

	// try it as an obj
	try {
		Object *target = get_obj_target(var);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", var);

		if (member_name == "name") return target->short_descr;

		throw Format::format("progs::dereference_variable: unknown variable member '%s'", member_name);
	}
	catch (String e) {
		throw;
	}
}

const String MobProgContext::
compute_function(const String& fn, const std::vector<String>& args) const {
	// for brevity
	String arg1 = args.size() > 0 ? args[0] : "";
	String arg2 = args.size() > 1 ? args[1] : "";

	if (!strcmp(fn, "rand"))
		return (number_percent() <= atoi(arg1)) ? "1" : "0";

	if (!strcmp(fn, "mudtime"))
		return Format::format("%d", Game::world().time.hour);

	if (!strcmp(fn, "get_state")) {
		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		const auto entry = target->mpstate.find(arg2);

		int state = 0;

		if (entry != target->mpstate.cend())
			state = entry->second;

		return Format::format("%d", state);
	}

	if (!strcmp(fn, "iscarrying")) {
		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		return (get_obj_carry(target, arg2) == nullptr) ? "0" : "1";
	}

	if (!strcmp(fn, "iswearing")) {
		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::Context::compute_function: not enough args '%s' and '%s' for '%s'", arg1, arg2, fn);

		return (get_obj_wear(target, arg2) == nullptr) ? "0" : "1";
	}

	if (!strcmp(fn, "ispc")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return !target->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isnpc")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return target->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isgood")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_GOOD(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isevil")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_EVIL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isneutral")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_NEUTRAL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isfight")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return target->fighting ? "1" : "0";
	}

	if (!strcmp(fn, "isimmort")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_IMMORTAL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "iskiller")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_KILLER(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isthief")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return IS_THIEF(target) ? "1" : "0";
	}

	if (!strcmp(fn, "ischarmed")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return affect::exists_on_char(target, affect::type::charm_person) ? "1" : "0";
	}

	if (!strcmp(fn, "isfollow")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return (target->master && target->master->in_room == target->in_room) ? "1" : "0";
	}

	if (!strcmp(fn, "ismaster")) { /* is $ their master? */
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return mob->master == target ? "1" : "0";
	}

	if (!strcmp(fn, "isleader")) { /* is $ their leader? */
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return mob->leader == target ? "1" : "0";
	}
	if (!strcmp(fn, "hitprcnt")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->hit / GET_MAX_HIT(target));
	}

	if (!strcmp(fn, "inroom")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return target->in_room->location.to_string();
	}

	if (!strcmp(fn, "sex")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", GET_ATTR_SEX(target));
	}

	if (!strcmp(fn, "position")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->position);
	}

	if (!strcmp(fn, "level")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->level);
	}

	if (!strcmp(fn, "class")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->guild);
	}

	if (!strcmp(fn, "goldamt")) {
		Character *target;
		if ((target = get_char_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->gold);
	}

	if (!strcmp(fn, "objtype")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->item_type);
	}

	if (!strcmp(fn, "objval0")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->value[0]);
	}

	if (!strcmp(fn, "objval1")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->value[1]);
	}

	if (!strcmp(fn, "objval2")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->value[2]);
	}

	if (!strcmp(fn, "objval3")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->value[3]);
	}

	if (!strcmp(fn, "objval4")) {
		Object *target;
		if ((target = get_obj_target(arg1)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

		return Format::format("%d", target->value[4]);
	}

	if (!strcmp(fn, "number")) {
		// try it as a character
		try {
			Character *target;
			if ((target = get_char_target(arg1)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

			return Format::format("%d", target->pIndexData->vnum);
		}
		// try it as an obj
		catch (String e) {
			Object *target;
			if ((target = get_obj_target(arg1)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

			return Format::format("%d", target->pIndexData->vnum);
		}
	}

	if (!strcmp(fn, "name")) {
		// try it as a character
		try {
			Character *target;
			if ((target = get_char_target(arg1)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

			return target->name;
		}
		// try it as an obj
		catch (String e) {
			Object *target;
			if ((target = get_obj_target(arg1)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", arg1, fn);

			return target->name;
		}
	}

	throw Format::format("progs::compute_function: unknown function name '%s'", fn);
}


/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void MobProgContext::
process_command(const String& cmnd) const {
	String buf;
	const char *str = cmnd.c_str();

	while (*str != '\0') {
		if (*str != '$') {
			buf += *str++;
			continue;
		}

		str++;
		buf += translate(*str);
		str++;
	}

	interpret(mob, buf);
	return;
}

} // namespace progs

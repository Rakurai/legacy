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
get_char_target(const std::unique_ptr<Symbol> sym) const {
	Character *target = nullptr;
	String var = sym->to_string();

	if (var.length() == 2)
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'i': target = (Character *)self;
			case 'b': target = ((Character *)self)->master;
			case 'n': target = actor;
			case 't': target = (Character *)vo;
			case 'r': target = rndm;
			default:
				throw Format::format("progs::get_char_target: invalid target '%s'", var);
		}

	if (target == nullptr)
		throw Format::format("progs::get_char_target: null target '%s'", var);
}

Object * MobProgContext::
get_obj_target(const std::unique_ptr<Symbol> sym) const {
	Object *target = nullptr;
	String var = sym->to_string();

	if (var.length() == 2)
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'o': target = obj;
			case 'p': target = (Object *)vo;
			default:
				throw Format::format("progs::get_obj_target: invalid target '%s'", var);
		}

	if (target == nullptr)
		throw Format::format("progs::get_obj_target: null target '%s'", var);
}

const String MobProgContext::
dereference_variable(String var, String member_name) const {
	// try it as a character
	try {
		Character *target = get_char_target(var);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", var);

		return access_member(target, member_name, can_see_char(mob, target));
	}
	catch (...) {}

	// try it as an obj
	try {
		Object *target = get_obj_target(var);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", var);

		return access_member(target, member_name, can_see_obj(mob, target));
	}
	catch (...) {}

	throw Format::format("progs::dereference_variable: bad target '%s'", var);
}

const String MobProgContext::
compute_function(const String& fn, const std::vector<std::unique_ptr<Symbol>>& arg_list) const {
	if (!strcmp(fn, "ismaster")) { /* is $ their master? */
		return mob->master == get_char_target(arg_list[0]) ? "1" : "0";
	}

	if (!strcmp(fn, "isleader")) { /* is $ their leader? */
		return mob->leader == get_char_target(arg_list[0]) ? "1" : "0";
	}

	// not found, try to find it in non-mob-specific commands
	return Context::compute_function(fn, arg_list);
}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void MobProgContext::
process_command(const String& cmnd) const {
	String buf, copy = cmnd;

	while (!copy.empty()) {
		if (copy[0] == '$') {
			// allows dereferencing of variables in this command, shouldn't allow execution of functions!
			buf += Symbol::parse(copy, "")->evaluate(*this);
		}
		else {
			buf += copy[0];
			copy.erase(0, 1);
		}
	}

	interpret(mob, buf);
	return;
}

} // namespace progs

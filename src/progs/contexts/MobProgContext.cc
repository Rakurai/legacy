#include "progs/contexts/MobProgContext.hh"
#include "progs/prog_table.hh"
#include "Character.hh"
#include "MobilePrototype.hh"
#include "Room.hh"
#include "random.hh"
#include "Game.hh"

namespace progs {
namespace contexts {

const Vnum MobProgContext::
vnum() const {
	return mob->pIndexData->vnum;
}

bool MobProgContext::
can_see(Character *ch) const {
	return can_see_char(mob, ch);
}

bool MobProgContext::
can_see(Object *obj) const {
	return can_see_obj(mob, obj);
}

bool MobProgContext::
can_see(Room *room) const {
	return can_see_in_room(mob, room);
}

bool MobProgContext::
self_is_garbage() const {
	return mob->is_garbage();
}

MobProgContext::
MobProgContext(progs::Type type, Character *mob) :
	Context(prog_table.find(type)->second.default_bindings),
	mob(mob) 
{
	// set variables
	set_var("self", data::Type::Character, mob);
	set_var("world", data::Type::World, &Game::world());
	set_var("room", data::Type::Room, mob->in_room);
	set_var("master", data::Type::Character, mob->master);

	int count = 0;
	Character *rndm;

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

	set_var("random", data::Type::Character, rndm);
}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void MobProgContext::
process_command(const String& cmnd) {
	String buf = expand_vars(cmnd);
	interpret(mob, buf);
	return;
}

} // namespace contexts
} // namespace progs

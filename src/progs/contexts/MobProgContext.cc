#include "progs/contexts/MobProgContext.hh"
#include "Character.hh"
#include "MobilePrototype.hh"
#include "Room.hh"
#include "random.hh"

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
self_is_garbage() const {
	return mob->is_garbage();
}

MobProgContext::
MobProgContext(Character *mob) :
	mob(mob) 
{
	add_var("i", mob);
	add_var("b", mob->master);

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

	if (rndm != nullptr)
		add_var("r", rndm);
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

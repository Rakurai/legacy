#include "progs/contexts/ObjProgContext.hh"
#include "progs/symbols/Symbol.hh"
#include "Object.hh"
#include "Character.hh"
#include "ObjectPrototype.hh"
#include "Room.hh"
#include "random.hh"

namespace progs {
namespace contexts {

const Vnum ObjProgContext::
vnum() const {
	return obj->pIndexData->vnum;
}

bool ObjProgContext::
can_see(Character *ch) const {
	return true;
}

bool ObjProgContext::
can_see(Object *obj) const {
	return true;
}

bool ObjProgContext::
self_is_garbage() const {
	return false;
//	return obj->is_garbage();
}

ObjProgContext::
ObjProgContext(Object *obj) :
	obj(obj) 
{
	add_var("self", data::Type::Object, obj);

	int count = 0;

	Room *room = obj->in_room;

	if (obj->carried_by)
		room = obj->carried_by->in_room;

	if (room != nullptr) {
		Character *rndm;

		/* get a random visable mortal player who is in the room with the obj */
		for (Character *vch = obj->in_room->people; vch; vch = vch->next_in_room) {
			if (!vch->is_npc()
			    &&  !IS_IMMORTAL(vch)) {
//			    &&  can_see_char(mob, vch)) {
				if (number_range(0, count) == 0)
					rndm = vch;

				count++;
			}
		}

		if (rndm != nullptr)
			add_var("random", data::Type::Character, rndm);
	}
}
/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void ObjProgContext::
process_command(const String& cmnd) {
	String buf, copy = cmnd;
	std::unique_ptr<symbols::Symbol> stack = symbols::parse(copy, bindings, "");

	std::cout << stack->print_stack().c_str() << std::endl;

	stack->execute(*this);
}

} // namespace contexts
} // namespace progs

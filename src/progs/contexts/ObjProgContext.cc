#include "progs/contexts/ObjProgContext.hh"
#include "progs/symbols/declare.hh"
#include "progs/symbols/Symbol.hh"
#include "progs/prog_table.hh"
#include "Object.hh"
#include "Character.hh"
#include "ObjectPrototype.hh"
#include "Room.hh"
#include "random.hh"
#include "Game.hh"

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
can_see(Room *room) const {
	return true;
}

bool ObjProgContext::
self_is_garbage() const {
	return false;
//	return obj->is_garbage();
}

Room * ObjProgContext::
in_room() const {
	if (obj == nullptr)
		return nullptr;

	Room *room = nullptr;

	if (obj->carried_by != nullptr)
		room = obj->carried_by->in_room;
	else
		room = obj->in_room;

	return room;
}

ObjProgContext::
ObjProgContext(progs::Type type, Object *obj) :
	Context(prog_table.find(type)->second.default_bindings),
	obj(obj) 
{
	// bind aliases
	aliases.emplace("self", data::Type::Object);
	aliases.emplace("room", data::Type::Room);
	aliases.emplace("world", data::Type::World);

	Room *room = obj->in_room;

	if (obj->carried_by)
		room = obj->carried_by->in_room;

	if (room == nullptr)
		throw String("object is not in a room and not carried by a character");

	int count = 0;
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

	set_var("random", data::Type::Character, rndm); // could be nullptr, that's ok
}
/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void ObjProgContext::
process_command(const String& cmnd) {
	String buf, copy = cmnd;
	std::unique_ptr<symbols::Symbol> stack = symbols::parse(copy, bindings, "");
	debug(*this, Format::format("execute:   %s", stack->print_stack()));
	stack->execute(*this);
}

} // namespace contexts
} // namespace progs

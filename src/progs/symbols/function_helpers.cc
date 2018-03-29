#include "String.hh"
#include "Room.hh"
#include "Character.hh"
#include "Object.hh"
#include "Exit.hh"
#include "act.hh"
#include "argument.hh"
#include "progs/contexts/Context.hh"

namespace progs {
namespace symbols {

void fn_helper_echo(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_ROOM, POS_RESTING, false, room);
}

void fn_helper_echo_at(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_VICT, POS_RESTING, false, room);
}

void fn_helper_echo_other(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_NOTVICT, POS_RESTING, false, room);
}

void fn_helper_echo_near(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	if (room == nullptr)
		return;

	for (int door = 0; door <= 5; door++) {
		Exit *pexit = room->exit[door];

		if (pexit != nullptr
		 && pexit->to_room != nullptr
		 && pexit->to_room != room)
			fn_helper_echo(format, actor, v1, v2, pexit->to_room);
	}
}

Character * fn_helper_get_char(const String& str, contexts::Context& context, Room *room) {
	String arg;
	int count = 0, number = number_argument(str, arg);

	if (arg.empty())
		return nullptr;

	for (auto ch = room->people; ch; ch = ch->next_in_room)
		if (context.can_see(ch)
		 && (ch->name.has_words(arg))
		 && ++count == number)
			return ch;

	return nullptr;
}

void fn_helper_junk(Character *ch, Object *obj) {
	if (obj == nullptr)
		return;

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	extract_obj(obj);
}

} // namespace symbols
} // namespace progs

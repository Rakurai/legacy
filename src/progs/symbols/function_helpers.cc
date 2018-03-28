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

void fn_helper_echo(const String& format, Room *room, Character *ch, Object *obj) {
	if (ch != nullptr || obj != nullptr) {
		// uses act() to distribute the message, but act variables don't match up with prog
		// variables, so we need to do variable replacement in the string and pass a straight
		// string to act.  maybe someday we can rework this and let act do the work.
		act(format, ch, obj, nullptr, TO_ROOM);
	}
	else if (room != nullptr) {
		for (Character *ch = room->people; ch; ch = ch->next_in_room)
			stc(format + "\n", ch);
	}
}


void fn_helper_echo_near(const String& format, Room *room) {
	for (int door = 0; door <= 5; door++) {
		Exit *pexit = room->exit[door];

		if (pexit != nullptr
		 && pexit->to_room != nullptr
		 && pexit->to_room != room)
			fn_helper_echo(format, room, nullptr, nullptr);
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

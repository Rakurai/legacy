#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "comm.hh"

namespace conn {

extern bool attempt_reconnect(Descriptor *d, const String& name);

State * BreakConnectState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;
	Descriptor *d_old, *d_next;

	switch (argument[0]) {
	case 'y':
	case 'Y':
		for (d_old = descriptor_list; d_old != nullptr; d_old = d_next) {
			d_next = d_old->next;

			if (d_old == d || d_old->character == nullptr)
				continue;

			if (ch->name != (d_old->original ?
			            d_old->original->name : d_old->character->name))
				continue;

			close_socket(d_old);
		}

		if (attempt_reconnect(d, ch->name))
			return &State::playing;

		ptc(ch, "Reconnect attempt failed.\nName: ");

		if (d->character != nullptr) {
			delete d->character;
			d->character = nullptr;
		}

		return &State::getName;

	case 'n':
	case 'N':
		ptc(ch, "Name: ");

		if (d->character != nullptr) {
			delete d->character;
			d->character = nullptr;
		}

		return &State::getName;

	default:
		ptc(ch, "Please type Y or N? ");
		break;
	}

	return this;
}

} // namespace conn

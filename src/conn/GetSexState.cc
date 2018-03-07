#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

State * GetSexState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	switch (argument[0]) {
	case 'm':
	case 'M':
		ATTR_BASE(ch, APPLY_SEX) = SEX_MALE;
		break;

	case 'f':
	case 'F':
		ATTR_BASE(ch, APPLY_SEX) = SEX_FEMALE;
		break;

	default:
		ptc(ch, "That's not a sex.\n(M/F)? ");
		return this;
	}

	ptc(ch, "\n");
	String buf = "Select a class [";

	for (int i = Guild::first; i < Guild::size; i++) {
		if (i > Guild::first)
			buf += " ";

		buf += guild_table[i].name;
	}

	buf += "]\nHelp file: class\nWhat is your class? ";
	ptc(ch, buf);

	return &State::getGuild;
}

} // namespace conn

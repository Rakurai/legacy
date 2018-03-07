#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

State * ReadIMOTDState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	ptc(ch, "\n");
	set_color(ch, CYAN, NOBOLD);
	help(ch, "automotd");
	set_color(ch, WHITE, NOBOLD);

	return &State::readMOTD;
}

} // namespace conn

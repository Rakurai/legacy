#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

void ReadIMOTDState::
transitionIn(Character *ch) {
	set_color(ch, RED, BOLD);
	help(ch, "imotd");
	set_color(ch, WHITE, NOBOLD);
}

State * ReadIMOTDState::
handleInput(Descriptor *d, const String& argument) {
	State::readMOTD.transitionIn(d->character);
	return &State::readMOTD;
}

} // namespace conn

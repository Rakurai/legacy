#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

void ReadMOTDState::
transitionIn(Character *ch) {
	ptc(ch, "\n");
	set_color(ch, CYAN, NOBOLD);
	help(ch, "automotd");
	set_color(ch, WHITE, NOBOLD);
}

State * ReadMOTDState::
handleInput(Descriptor *d, const String& argument) {
	State::readNewMOTD.transitionIn(d->character);
	return &State::readNewMOTD;
}

} // namespace conn

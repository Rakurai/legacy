#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Game.hh"

namespace conn {

State * ReadMOTDState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	set_color(ch, WHITE, BOLD);

	if (!Game::motd.empty())
		stc(Game::motd, ch);

	stc("\n{x[Hit Enter to continue]", ch);
	set_color(ch, WHITE, NOBOLD);

	return &State::readNewMOTD;
}

} // namespace conn

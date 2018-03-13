#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Game.hh"

namespace conn {

void ReadNewMOTDState::
transitionIn(Character *ch) {
	set_color(ch, WHITE, BOLD);

	if (!Game::motd.empty())
		stc(Game::motd, ch);

	stc("\n{x[Hit Enter to continue]", ch);
	set_color(ch, WHITE, NOBOLD);
}

State * ReadNewMOTDState::
handleInput(Descriptor *d, const String& argument) {
	State::playing.transitionIn(d->character);
	return &State::playing;
}

} // namespace conn

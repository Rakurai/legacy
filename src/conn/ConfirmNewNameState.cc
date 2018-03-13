#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

void ConfirmNewNameState::
prompt(Character *ch) {
	ptc(ch, "\n{YAre you sure? (Yes/No):{x ");
}

void ConfirmNewNameState::
transitionIn(Character *ch) {
	help(ch, "creation_confirm_new_name");
	prompt(ch);
}

State * ConfirmNewNameState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	/* We're past all the reconnect stuff, we can be sure that the character
	   is not playing and is new.  Start using stc and ptc for the colors */
	switch (argument[0]) {
	case 'y':
	case 'Y':
		State::getNewPass.transitionIn(ch);
		return &State::getNewPass;

	case 'n':
	case 'N':
		help(ch, "creation_confirm_new_name_goback");
		State::getNewName.prompt(ch);
		return &State::getNewName;

	default:
		prompt(ch);
		break;
	}

	return this;
}

} // namespace conn

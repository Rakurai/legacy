#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "argument.hh"
#include "comm.hh"
#include "merc.hh"

namespace conn {

void ConfirmNewPassState::
prompt(Character *ch) {
	ptc(ch, "\n{YEnter the same password again:{x ");
	echo_off(ch->desc);
}

void ConfirmNewPassState::
transitionIn(Character *ch) {
	help(ch, "creation_confirm_new_pass");
	prompt(ch);
}

State * ConfirmNewPassState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	echo_on(d);

	String pass;
	one_argument(argument, pass);

	if (strcmp(pass, ch->pcdata->pwd)) {
		help(ch, "creation_confirm_new_pass_goback");
		State::getNewPass.prompt(ch);
		return &State::getNewPass;
	}

	State::playing.transitionIn(ch);
	return &State::playing;
//	State::rollStats.transitionIn(ch);
//	return &State::rollStats;
}

} // namespace conn

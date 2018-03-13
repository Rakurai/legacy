#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "argument.hh"
#include "comm.hh"

namespace conn {

void GetNewPassState::
prompt(Character *ch) {
	ptc(ch, "\n{YEnter a password:{x ");
	echo_off(ch->desc);
}

void GetNewPassState::
transitionIn(Character *ch) {
	help(ch, "creation_get_new_pass_1");
	ptc(ch, ch->name);
	help(ch, "creation_get_new_pass_2");
	prompt(ch);
}

State * GetNewPassState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	echo_on(d);

	String pass;
	one_argument(argument, pass);

	if (pass.length() < 5) {
		help(ch, "creation_get_new_pass_error");
		prompt(ch);
		return this;
	}

	ch->pcdata->pwd = pass;
	State::confirmNewPass.transitionIn(ch);
	return &State::confirmNewPass;
}

} // namespace conn

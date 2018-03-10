#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "argument.hh"

namespace conn {

State * GetNewPassState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	String pass;
	one_argument(argument, pass);

	stc("\n", ch);

	if (pass.length() < 5) {
		stc("You whisper in her ear, and she giggles.\n\n"
		    "{Y'{WThat's too short, somebody could guess it easily.  Make up one that's at"
		    " least five letters long!{Y'{x she says, and flies up close to hear you whisper.\n\n", ch);
		stc("{CWhat password do you tell her?{x ", ch);
		return this;
	}

	ch->pcdata->pwd = pass;
	stc("The pixie smiles as you whisper in her ear.\n\n"
	    "{Y'{WGood, that will do nicely.  Just to make sure I have it, say it one more time.{Y'{x\n\n", ch);
	stc("{CPlease repeat the password.{x ", ch);

	return &State::confirmNewPass;
}

} // namespace conn

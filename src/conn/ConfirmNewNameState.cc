#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"

namespace conn {

State * ConfirmNewNameState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	/* We're past all the reconnect stuff, we can be sure that the character
	   is not playing and is new.  Start using stc and ptc for the colors */
	switch (argument[0]) {
	case 'y':
	case 'Y':
		stc("\nYou find yourself standing in the market square of Midgaard, surrounded by\n"
		    "the exotic scents and bright colors of the open bazaar.  A tiny pixie flits\n"
		    "through the air and hovers in front of you, looking you over approvingly.\n\n", ch);
		ptc(ch, "{Y'{WWelcome to Legacy, %s!{Y'{x she says, smiling happily.  {Y'{WI hope you have\n"
		    " some free time, the world is a big place and there is so much to do!{Y'{x\n\n", ch->name);
		stc("{Y'{WFirst things first, though!  Tell me, how familiar are you with Legacy?{Y'{x\n\n", ch);
		stc("{Y1){x ... what is this place?                  ({YNew to MUDs{x)\n"
		    "{Y2){x I've never been to this realm.           ({YNew to Legacy{x)\n"
		    "{Y3){x Let's go, I know everything!             ({YExperienced player{x)\n\n"
		    "{CPlease answer {Y1{C, {Y2{C, or {Y3{C.{x ", ch);

		return &State::getMudExp;

	case 'n':
	case 'N':
		stc("Ok, what do you want to be called, then? ", ch);
		delete d->character;
		d->character = nullptr;

		return &State::getName;

	default:
		stc("Please type Yes or No? ", ch);
		break;
	}

	return this;
}

} // namespace conn

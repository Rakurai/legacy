#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "comm.hh"

namespace conn {

State * GetMudExpState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	switch (argument[0]) {
	case '1':
		stc("\nThe pixie beams a smile at you, and you shuffle your feet nervously.\n\n"
		    "{Y'{WWell then,{Y'{x she says, {Y'{Wyou've been missing out!  Legacy is a MUD, a place\n"
		    " where you can walk, talk, and interact with people all over the world in a\n"
		    " fantasy setting.  You've taken the first step, as we all did.  I will warn\n"
		    " you, though, this place is addicting, you may not want to leave.{Y'{x  She winks\n"
		    " at you, and you find yourself following her toward a massive white marble\n"
		    " temple just north of the marketplace.\n\n", ch);
		stc("{Y'{WBefore we continue, we need to tell your password to the city guards.\n"
		    " Every time you come back to visit us, you will have to speak your password\n"
		    " before they will let you in.  Think of a good one, so nobody can pretend\n"
		    " to be you!  Just whisper it in my ear, and we'll make sure they record it.{Y'{x\n\n", ch);
		ch->pcdata->mud_exp = MEXP_TOTAL_NEWBIE;
		break;

	case '2':
		stc("\nThe pixie beams a smile at you.\n\n"
		    "{Y'{WAllow me, then, to show you around!  We'll get you acquainted in no time.{Y'{x\n\n"
		    "You start to follow her toward a massive white marble temple just north of\n"
		    "the marketplace.\n\n"
		    "{Y'{WBefore we continue, we need to tell your password to the city guards, so\n"
		    " they will let you in when you come back to visit us.  Just whisper it in my\n"
		    " ear, and we'll make sure they record it.{Y'{x\n\n", ch);
		ch->pcdata->mud_exp = MEXP_LEGACY_NEWBIE;
		break;

	case '3':
		stc("\nThe pixie grins at you.\n\n"
		    "{Y'{WWell, then, there is no time to lose!  I'm sure you know how strict the\n"
		    " city guards can be, let's give them your password.  Whisper it in my ear,\n"
		    " we'll make sure they record it.{Y'{x\n\n", ch);
		ch->pcdata->mud_exp = MEXP_LEGACY_OLDBIE;
		break;

	default:
		stc("{CPlease answer {Y1{C, {Y2{C, or {Y3{C.{x ", ch);
		return this;
	}

	ptc(ch, "{CWhat password do you tell her?{x ");
	echo_off(d);
	return &State::getNewPass;
}

} // namespace conn

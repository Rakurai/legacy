#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "argument.hh"
#include "comm.hh"
#include "merc.hh"

namespace conn {

State * ConfirmNewPassState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	String pass;
	one_argument(argument, pass);

	stc("\n", ch);

	if (strcmp(pass, ch->pcdata->pwd)) {
		stc("The pixie frowns as you whisper again.\n\n"
		    "{Y'{WThat does not sound like the first password you told me... let's try again.\n"
		    " What password do you want to use?{Y'{x she says as she flies closer to hear.\n\n", ch);
		stc("{CWhat password do you tell her?{x ", ch);

		return &State::getNewPass;
	}

	echo_on(d);
	ptc(ch, "She nods her approval.\n\n");
	ptc(ch, "{Y'{WYes, that's what I thought you said.{Y'{x  As your walk draws close to the grand\n"
	    " marble steps leading to the temple, she flies over to a nearby cityguard and\n"
	    " taps him on the ear.  {Y'{WGood morning!{Y'{x she says.  {Y'{W%s here is a friend of\n"
	    " mine, and needs to be added to the list of citizens.  The password is...{Y'{x She\n"
	    " flies close to the cityguard's ear and whispers, then flits back to your side.\n\n", ch->name);
	stc("She leads you to the first of the temple steps, and lands gracefully on the\n"
	    "carved handrail.  She gestures for you to take a seat.\n\n"
	    "{Y'{WThe temple ahead is a place of worship, and also a place of learning.\n"
	    " Before we go there, though, tell me about yourself.{Y'{x  She smiles.  {Y'{WI can\n"
	    "see that you are a...{Y'{x\n\n", ch);
	stc("Here you may choose your race.  If you are unsure of what you would like\n"
	    "to be, just type {Yhelp{x for information on the races.\n\n", ch);
	stc("The following races are available:\n", ch);

	for (unsigned int race = 1; race < race_table.size() && race_table[race].pc_race; race++)
		if (!pc_race_table[race].remort_level)
			ptc(ch, "%s\n", race_table[race].name);

	stc("\n{CWhat is your race?{x ", ch);

	return &State::getRace;
}

} // namespace conn

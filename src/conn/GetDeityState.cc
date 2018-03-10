#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Player.hh"
#include "argument.hh"
#include "lookup.hh"
#include "merc.hh"

namespace conn {

State * GetDeityState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	if (argument.has_prefix("help")) {
		String arg1, arg2;
		arg2 = one_argument(argument, arg1);

		if (arg2.empty())
			help(ch, "deity");
		else
			help(ch, arg2);

		ptc(ch, "Who is your deity? ");
		return this;;
	}

	int deity = deity_lookup(argument);

	if (deity == -1) {
		ptc(ch, "That's not a valid deity.\nWho is your deity? ");
		return this;
	}

	ch->pcdata->deity = deity_table[deity].name;

	if (ch->pcdata->points < 40)
		ch->train = 40 - ch->pcdata->points;

	ptc(ch, "\n");
	ptc(ch, "Please pick a weapon from the following choices:\n");
	String buf;

	for (const auto& entry : weapon_table)
		if (get_learned(ch, entry.skill) > 0) {
			buf += entry.name;
			buf += " ";
		}

	buf += "\nYour choice? ";
	ptc(ch, buf);

	return &State::getWeapon;
}

} // namespace conn

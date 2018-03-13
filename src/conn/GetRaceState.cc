#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "argument.hh"
#include "Character.hh"
#include "lookup.hh"
#include "merc.hh"
#include "affect/Affect.hh"

namespace conn {

void GetRaceState::
prompt(Character *ch) {
	stc("\n{YWhat is your race?{x ", ch);
}

void GetRaceState::
transitionIn(Character *ch) {
	help(ch, "creation_ask_race");

	for (unsigned int race = 1; race < race_table.size() && race_table[race].pc_race; race++)
		if (!pc_race_table[race].remort_level)
			ptc(ch, "%s ", race_table[race].name);

	ptc(ch, "\n");
	prompt(ch);
}

State * GetRaceState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	if (argument.has_prefix("help")) {
		String arg;
		String arg2 = one_argument(argument, arg);

		if (arg2.empty())
			help(ch, "races");
		else
			help(ch, arg2);

		prompt(ch);
		return this;
	}

	int race = race_lookup(argument);

	if (race < 1
	 || !race_table[race].pc_race
	 || pc_race_table[race].remort_level) {
		ptc(ch, "That is not a valid race.\n");
		prompt(ch);
		return this;
	}

	ch->race = race;

	/* initialize stats */
	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(ch, stat_to_attr(stat)) = pc_race_table[race].stats[stat];

	affect::add_racial_to_char(ch);

	ch->form_flags                = race_table[race].form;
	ch->parts_flags               = race_table[race].parts;
	ch->pcdata->points      = pc_race_table[race].points;
	ch->size                = pc_race_table[race].size;

	/* add skills */
	for (int i = 0; i < 5 && !pc_race_table[race].skills[i].empty(); i++)
		group_add(ch, pc_race_table[race].skills[i], false);

	State::getNewName.transitionIn(ch);
	return &State::getNewName;

/*
	ptc(ch, "\n");
	ptc(ch, "Here are your default stats:\n");
	ptc(ch, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
	        ATTR_BASE(ch, APPLY_STR), ATTR_BASE(ch, APPLY_INT),
	        ATTR_BASE(ch, APPLY_WIS), ATTR_BASE(ch, APPLY_DEX),
	        ATTR_BASE(ch, APPLY_CON), ATTR_BASE(ch, APPLY_CHR));
	ptc(ch, "Would you like to roll for new stats? [Y/N] ");

	return &State::rollStats;
*/
}

} // namespace conn

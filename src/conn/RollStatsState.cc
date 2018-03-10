#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "random.hh"
#include "merc.hh"

namespace conn {

int roll_stat(const Character *ch, int stat)
{
	int percent, bonus, temp, low, high;
	percent = number_percent();

	if (percent > 99)
		bonus = 2;
	else if (percent > 95)
		bonus = 1;
	else if (percent < 5)
		bonus = -1;
	else
		bonus = 0;

	high = pc_race_table[ch->race].max_stats[stat] - (3 - bonus);
	low = pc_race_table[ch->race].stats[stat] - (3 - bonus);
	temp = (number_range(low, high));
	return temp;
}

State * RollStatsState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	switch (argument[0]) {
	case 'n':
	case 'N':
		ptc(ch, "\n");
		ptc(ch, "What is your sex (M/F)? ");

		return &State::getSex;

	case 'y':
	case 'Y':
		for (int stat = 0; stat < MAX_STATS; stat++)
			ATTR_BASE(ch, stat_to_attr(stat)) = roll_stat(ch, stat);

		ptc(ch, "\nStr: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
		        ATTR_BASE(ch, APPLY_STR), ATTR_BASE(ch, APPLY_INT),
		        ATTR_BASE(ch, APPLY_WIS), ATTR_BASE(ch, APPLY_DEX),
		        ATTR_BASE(ch, APPLY_CON), ATTR_BASE(ch, APPLY_CHR));
		ptc(ch, "Would you like to roll for new stats? [Y/N] ");
		break;

	default:
		ptc(ch, "Yes or No? ");
		break;
	}

	return this;
}

} // namespace conn

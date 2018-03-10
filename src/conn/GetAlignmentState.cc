#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "merc.hh"

namespace conn {

State * GetAlignmentState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	switch (argument[0]) {
	case 'g':
	case 'G':
		if (ch->guild == Guild::paladin)
			ch->alignment = 1000;
		else
			ch->alignment = 750;

		break;

	case 'e':
	case 'E':
		if (ch->guild == Guild::paladin)
			ch->alignment = -1000;
		else
			ch->alignment = -750;

		break;

	case 'n':
	case 'N':

		/* paladins drop through to default */
		if (ch->guild != Guild::paladin) {
			ch->alignment = 0;
			break;
		}

	default:
		ptc(ch, "That's not a valid alignment.\nWhich alignment (G%s/E)? ",
		        ch->guild == Guild::paladin ? "" : "/N");

		return this;
	}

	group_add(ch, "rom basics", false);
	group_add(ch, guild_table[ch->guild].base_group, false);
	set_learned(ch, skill::type::recall, 50);
	set_learned(ch, skill::type::scan, 100);
	String buf = "\nSelect a deity:\n";

	for (const auto& entry : deity_table) {
		if (ch->guild == Guild::paladin) { /* Paladins */
			if (entry.value > 0 && ch->alignment > 0) {
				buf += entry.align;
				buf += entry.name;
				buf += "\n";
			}
			else if (entry.value < 0 && ch->alignment < 0) {
				buf += entry.align;
				buf += entry.name;
				buf += "\n";
			}
		}
		else if (entry.value == ch->alignment
		         ||       entry.value == -1) {
			buf += entry.align;
			buf += entry.name;
			buf += "\n";
		}
	}

	ptc(ch, buf);
	ptc(ch, "\nHelp file: deity\nWho is your deity? ");

	return &State::getDeity;
}

} // namespace conn

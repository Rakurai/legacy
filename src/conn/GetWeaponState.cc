#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "lookup.hh"

namespace conn {

State * GetWeaponState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	ptc(ch, "\n");
	int weapon = weapon_lookup(argument);

	if (weapon == -1 || get_learned(ch, weapon_table[weapon].skill) <= 0) {
		ptc(ch, "That is not a valid selection.  Your choice? \n");
		return this;
	}

	set_learned(ch, weapon_table[weapon].skill, 40);
	ptc(ch, "\n");

	set_color(ch, CYAN, BOLD);
	help(ch, "automotd");
	set_color(ch, WHITE, NOBOLD);

	return &State::readMOTD;
}

} // namespace conn

#include "Room.hh"
#include "Character.hh"
#include "Area.hh"
#include "Object.hh"
#include "Logging.hh"

void Room::
add_char(Character *ch) {
	ch->in_room         = this;
	ch->next_in_room    = this->people;
	this->people        = ch;

	area().add_char(ch);

	Object *obj;
	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != nullptr
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0)
		++this->light;

	const affect::Affect *plague = affect::find_on_char(ch, affect::type::plague);
	if (plague)
		spread_plague(this, plague, 6);
}

void Room::
remove_char(Character *ch) {
	Object *obj;
	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != nullptr
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0
	    &&   this->light > 0)
		--this->light;

	area().remove_char(ch);

	if (ch == this->people)
		this->people = ch->next_in_room;
	else {
		Character *prev;

		for (prev = this->people; prev; prev = prev->next_in_room) {
			if (prev->next_in_room == ch) {
				prev->next_in_room = ch->next_in_room;
				break;
			}
		}

		if (prev == nullptr) {
			Logging::bug("Char_from_room: ch not found.", 0);
			Logging::bug(ch->name, 0);
		}
	}

	ch->in_room      = nullptr;
	ch->next_in_room = nullptr;
	ch->on           = nullptr;  /* sanity check! */
}

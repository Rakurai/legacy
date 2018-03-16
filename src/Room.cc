#include "Room.hh"
#include "Character.hh"
#include "Area.hh"
#include "Object.hh"
#include "Logging.hh"
#include "RoomPrototype.hh"

Room::
Room(RoomPrototype& p) : 
	prototype(p), 
	room_flags(p.room_flags), 
	location(RoomID(p.vnum, ++p.count)) // increment the count for the prototype
{}

Area& Room::area() const { return prototype.area; }
const String& Room::name() const { return prototype.name; }
const String& Room::description() const { return prototype.description; }
const ExtraDescr *Room::extra_descr() const { return prototype.extra_descr; }
const Flags Room::flags() const { return cached_room_flags + room_flags; }
Sector Room::sector_type() const { return prototype.sector_type; }
int Room::guild() const { return prototype.guild; }
const String& Room::owner() const { return prototype.owner; }
const Clan *Room::clan() const { return prototype.clan; }
int Room::heal_rate() const { return prototype.heal_rate; }
int Room::mana_rate() const { return prototype.mana_rate; }
const Location& Room::tele_dest() const { return prototype.tele_dest; }

bool Room::is_on_map() const { return location.coord.is_valid(); }

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
	// make sure they're actually in this room first.  The only place I can find
	// where this might not happen is in do_load, but there's no sense in not
	// checking in case someone does something dumb.  I want this light thing
	// bulletproof. -- Montrey
	for (auto rch = people; true; rch = rch->next_in_room) {
		// weird looking loop will also handle the ch==nullptr case

		if (rch == nullptr) // end of loop, not found
			return;

		if (rch == ch) // got em, move on down
			break;
	}

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

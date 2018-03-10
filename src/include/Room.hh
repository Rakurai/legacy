#pragma once

#include "String.hh"
#include "Flags.hh"
#include "Location.hh"
#include "Sector.hh"

class RoomPrototype;
class Character;
class Object;
class Exit;
namespace affect { class Affect; }

class Room
{
public:
	Room(RoomPrototype& p);
	virtual ~Room() {}

	Character *		people = nullptr;
	Object *		contents = nullptr;
	Exit *		exit    [6] = {nullptr};

	affect::Affect *		affected = nullptr;		/* Montrey */
    Flags           cached_room_flags = 0;

	int			light = 0;
	int			hunt_id = 0;  /* Unique ID for current hunt */
	Room *	hunt_next = nullptr;  /* next room in search circle */
	Room *	hunt_back = nullptr;  /* pointer back toward origin */

//	const Vnum& vnum() const { return prototype.vnum; }
	Area& area() const;
	const String& name() const;
	const String& description() const;
	const ExtraDescr *extra_descr() const;
	const Flags flags() const;
	Sector sector_type() const;
	int guild() const;
	const String& owner() const;
	const Clan *clan() const;
	int heal_rate() const;
	int mana_rate() const;
	const Location& tele_dest() const;

	bool is_on_map() const;

	// modifiers
	void add_char(Character *ch);
	void remove_char(Character *ch);

	RoomPrototype& prototype;
    Flags           room_flags; // copied from the prototype for room-specific things, such as NO_MOB from fix_exits
    Location       location;

private:
	Room(const Room&);
	Room& operator=(const Room&);
};

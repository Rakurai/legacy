#pragma once

#include "affect/Affect.hh"
#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "RoomPrototype.hh"
#include "worldmap/Coordinate.hh"

class Room
{
public:
	Room(const RoomPrototype& p) : 
		prototype(p), room_flags(p.room_flags) {}
	Room(const RoomPrototype& p, const worldmap::Coordinate& c) : 
		prototype(p), room_flags(p.room_flags), coord(c) {}
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

	const Vnum& vnum() const { return prototype.vnum; }
	const Area& area() const { return prototype.area; }
	const String& name() const { return prototype.name; }
	const String& description() const { return prototype.description; }
	const ExtraDescr *extra_descr() const { return prototype.extra_descr; }
	const Flags flags() const { return cached_room_flags + room_flags; }
	Sector sector_type() const { return prototype.sector_type; }
	int guild() const { return prototype.guild; }
	const String& owner() const { return prototype.owner; }
	const Clan *clan() const { return prototype.clan; }
	int heal_rate() const { return prototype.heal_rate; }
	int mana_rate() const { return prototype.mana_rate; }
	const Vnum& tele_dest() const { return prototype.tele_dest; }

	bool is_on_map() const { return coord.is_valid(); }

	const RoomPrototype& prototype;
    Flags           room_flags; // copied from the prototype for room-specific things, such as NO_MOB from fix_exits
	worldmap::Coordinate coord;

private:
	Room(const Room&);
	Room& operator=(const Room&);
};

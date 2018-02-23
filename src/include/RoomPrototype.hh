#pragma once

#include "affect/Affect.hh"
#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "Vnum.hh"
#include "Sector.hh"
#include "Location.hh"

class RoomPrototype
{
public:
	RoomPrototype(Area&, const Vnum&, FILE *);
	virtual ~RoomPrototype();

	Area&		area;
	const Vnum  vnum;
	String 		name;
	String 		description;
	const Location tele_dest;
	Flags       room_flags;
	Sector		sector_type;
	int			heal_rate = 100;
	int			mana_rate = 100;
	int			guild = 0;		/* guild room, class number+1, 0 none -- Montrey */

	String 		owner = "";
	Clan *		clan = nullptr;
	int         count = 0; // number of rooms created from this prototype, incremented by Room

	// pointers owned here
	ExtraDescr *	extra_descr = nullptr;
	ExitPrototype *		exit    [6] = {nullptr}; // prototypes for building rooms

	inline bool operator==(const RoomPrototype& rhs) const {
		return this->vnum == rhs.vnum;
	}

private:
	RoomPrototype(const RoomPrototype&);
	RoomPrototype& operator=(const RoomPrototype&);
};


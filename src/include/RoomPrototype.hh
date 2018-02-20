#pragma once

#include "affect/Affect.hh"
#include "declare.hh"
#include "String.hh"
#include "Flags.hh"

class RoomPrototype
{
public:
	RoomPrototype() {}
	virtual ~RoomPrototype() {}

	Character *		people = nullptr;
	Object *		contents = nullptr;
	ExtraDescr *	extra_descr = nullptr;
	Area *		area = nullptr;
	Exit *		exit    [6] = {nullptr};
	Exit *		old_exit[6] = {nullptr};
	String 		name;
	String 		description;
	String 		owner;
	int			vnum = 0;
	int			version = 0;	/* Room versioning -- Montrey */
	affect::Affect *		affected = nullptr;		/* Montrey */

	Flags           room_flags;
    Flags           cached_room_flags;

	int			light = 0;
	int			sector_type = 0;
	int			heal_rate = 0;
	int			mana_rate = 0;
	Clan *		clan = nullptr;
	int			guild = 0;		/* guild room, class number+1, 0 none -- Montrey */
	int			tele_dest = 0;
	int			hunt_id = 0;  /* Unique ID for current hunt */
	RoomPrototype *	hunt_next = nullptr;  /* next room in search circle */
	RoomPrototype *	hunt_back = nullptr;  /* pointer back toward origin */

private:
	RoomPrototype(const RoomPrototype&);
	RoomPrototype& operator=(const RoomPrototype&);
};

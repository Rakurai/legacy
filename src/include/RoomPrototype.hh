#pragma once

#include "declare.hh"
#include "String.hh"
#include "Flags.hh"

class RoomPrototype
{
public:
	RoomPrototype() {}
	virtual ~RoomPrototype() {}

	RoomPrototype *	next = nullptr;
	Character *		people = nullptr;
	Object *		contents = nullptr;
	ExtraDescr *	extra_descr = nullptr;
	Area *		area = nullptr;
	Exit *		exit    [6] = {nullptr};
	Exit *		old_exit[6] = {nullptr};
	String 		name;
	String 		description;
	String 		owner;
	sh_int			vnum = 0;
	sh_int			version = 0;	/* Room versioning -- Montrey */
	Affect *		affected = nullptr;		/* Montrey */

	Flags           room_flags;
    Flags           cached_room_flags;

	sh_int			light = 0;
	sh_int			sector_type = 0;
	sh_int			heal_rate = 0;
	sh_int			mana_rate = 0;
	Clan *		clan = nullptr;
	sh_int			guild = 0;		/* guild room, class number+1, 0 none -- Montrey */
	sh_int			tele_dest = 0;
	sh_int			hunt_id = 0;  /* Unique ID for current hunt */
	RoomPrototype *	hunt_next = nullptr;  /* next room in search circle */
	RoomPrototype *	hunt_back = nullptr;  /* pointer back toward origin */

private:
	RoomPrototype(const RoomPrototype&);
	RoomPrototype& operator=(const RoomPrototype&);
};

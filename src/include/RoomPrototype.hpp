#pragma once

class RoomPrototype
{
public:
	RoomPrototype() {}
	virtual ~RoomPrototype() {}

	RoomPrototype *	next = NULL;
	Character *		people = NULL;
	Object *		contents = NULL;
	ExtraDescr *	extra_descr = NULL;
	Area *		area = NULL;
	Exit *		exit    [6] = {NULL};
	Exit *		old_exit[6] = {NULL};
	String 		name;
	String 		description;
	String 		owner;
	sh_int			vnum = 0;
	sh_int			version = 0;	/* Room versioning -- Montrey */
	Affect *		affected = NULL;		/* Montrey */

	unsigned long   room_flags = 0;
    unsigned long   room_flag_cache = 0;

	sh_int			light = 0;
	sh_int			sector_type = 0;
	sh_int			heal_rate = 0;
	sh_int			mana_rate = 0;
	Clan *		clan = NULL;
	sh_int			guild = 0;		/* guild room, class number+1, 0 none -- Montrey */
	sh_int			tele_dest = 0;
	sh_int			hunt_id = 0;  /* Unique ID for current hunt */
	RoomPrototype *	hunt_next = NULL;  /* next room in search circle */
	RoomPrototype *	hunt_back = NULL;  /* pointer back toward origin */

private:
	RoomPrototype(const RoomPrototype&);
	RoomPrototype& operator=(const RoomPrototype&);
};

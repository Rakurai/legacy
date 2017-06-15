#pragma once

#include "declare.hh"
#include "memory.hh"

/* vals from db.c */
extern bool fBootDb;
extern MobilePrototype 	* mob_index_hash          [MAX_KEY_HASH];
extern ObjectPrototype 	* obj_index_hash          [MAX_KEY_HASH];
extern RoomPrototype	* room_index_hash	[MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed;
extern sh_int		aVersion;

void boot_bug(const String& str, int param);

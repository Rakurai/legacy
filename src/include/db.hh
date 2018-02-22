#pragma once

#include <map>

#include "declare.hh"
#include "memory.hh"

/* vals from db.c */
extern bool fBootDb;
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed;
extern int		aVersion;

void boot_bug(const String& str, int param);
void boot_bug(const String& str, const Vnum& param);

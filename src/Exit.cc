#include "Exit.hh"
#include "merc.hh"

Exit::
Exit(const ExitPrototype& proto) :
	prototype(proto)
{
	to_room = get_room(prototype.to_vnum); // could be nullptr, deal with it outside
}

#include "Exit.hh"
#include "merc.hh"

Exit::
Exit(const ExitPrototype& proto, Room *to_room) :
	prototype(proto),
	exit_flags(proto.exit_flags),
	to_room(to_room)
{
}

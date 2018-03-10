#include "Exit.hh"
#include "ExitPrototype.hh"

Exit::
Exit(const ExitPrototype& proto, Room *to_room) :
	prototype(proto),
	exit_flags(proto.exit_flags),
	to_room(to_room)
{
}

int Exit::
key() const {
	return prototype.key;
}

const String& Exit::
keyword() const {
	return prototype.keyword;
}

const String& Exit::
description() const {
	return prototype.description;
}


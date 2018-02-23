#include "worldmap/Coordinate.hh"
#include "String.hh"
#include "Format.hh"

const String worldmap::Coordinate::
to_string(bool short_loc) const {
	if (!is_valid())
		return "(null)";

	String buf;

	if (short_loc)
		buf += Format::format("%d,%d", x, y);
	else
		buf += Format::format("%4d,%4d", x, y);

	return buf;
}

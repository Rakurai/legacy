#pragma once

#include "Flags.hh"
#include "Vnum.hh"
#include "worldmap/Coordinate.hh"
#include "RoomID.hh"

/* Overly complex looking Location type.
 * Needs to do a few things:
 *   uniquely identify a Room object
 *   store both a coordinate pair and an index/vnum combination
 *   be comparable to either a coordinate pair or an index/vnum combination
 *   be constructable from a coordinate pair, index/vnum combination, both
 *   be constructable from a String containing any legal representation
 *   implement operator< so that it can be used as a key in a std::map
 *   be serializable as a 32-bit integer representing either type
 *   be deserializable from a 32-bit integer representing either type
 *
 * To do this, we internally use a 64-bit signed long.  We reserve the first
 * bit (indicating negative numbers) to represent an invalid location, which
 * is the default constructed object and indicates a failure to translate
 * from the parameters.
 *
 * The next 3 bits are internally used flags, described below.  Since a
 * coordinate representation could be all 0s, we set a flag to indicate a
 * valid coordinate (even 0,0).  Other flags are reserved for future use.
 * When we serialize to a 32-bit int, we preserve the first 4 bits and
 * use either the coordinate date (dominant) or vnum data in the last 28 bits.
 *
 * The next 28 bits are coordinate data, 14 bits each for x and y.
 *
 * The next 4 bits are unused, because we limit either data type to 28 bits.
 * 
 * The last 28 bits are vnum data, 10 bits for the index (which child of the
 * prototype) and 18 bits for the vnum.
 */

class Location {
public:
	// invalid is the default, for things like pcdata->mark_room
	Location() : coord(), room_id() {}
	virtual ~Location() {} // no data to delete

	// allow copying Location objects
	Location(const Location& l) : coord(l.coord), room_id(l.room_id) {}
	Location& operator=(const Location& l) {
		coord = l.coord;
		room_id = l.room_id;
		return *this;
	}

	// rebuild from the serialized version, such as an object value or reset
	// deserialize both, one of them will be invalid
	explicit Location(int value) : coord(value), room_id(value) {}

	// construct from an index/vnum pair
	explicit Location(const RoomID& id) : coord(), room_id(id) {}

	// construct from a coordinate
	explicit Location(const worldmap::Coordinate& c) : coord(c), room_id() {}

	// construct from both coordinate and vnum
	explicit Location(const worldmap::Coordinate& c, const RoomID& id) : coord(c), room_id(id) {}

	// attempt to interpret a string as a location, invalid on failure
	explicit Location(const String& s) : Location() {
		// try it as a coordinate
		coord = worldmap::Coordinate(s);

		if (!coord.is_valid())
			room_id = RoomID(s);
	}

	// for storing in objects, resets, etc
	int to_int() const {
		if (coord.is_valid())
			return coord.to_int();

		return room_id.to_int();
	}

	// convert to string.  this will be a coordinate pair if possible, a vnum if not, or 0 if invalid
	const String to_string(bool short_loc = true) const {
		if (coord.is_valid())
			return coord.to_string(short_loc);

		return room_id.to_string(short_loc);
	}

	bool is_valid() const { return coord.is_valid() || room_id.is_valid(); }

	inline friend bool operator==(const Location& lhs, const Location& rhs) {
		// if both invalid, they're equal
		if (!lhs.is_valid() && !rhs.is_valid())
			return true;

		// otherwise one is valid, compare the data
		if (lhs.coord.is_valid()
		 && lhs.coord == rhs.coord)
			return true;

		if (lhs.room_id.is_valid()
		 && lhs.room_id == rhs.room_id)
			return true;

		return false;
	}

	inline friend bool operator!=(const Location& lhs, const Location& rhs) {
		return !(lhs == rhs);
	}

	worldmap::Coordinate coord;
	RoomID room_id;
};

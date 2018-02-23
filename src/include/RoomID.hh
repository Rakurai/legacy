#pragma once

#include "declare.hh"

class RoomID {
public:
	RoomID() : number_data(-1), vnum_data(0) {}
	RoomID(const Vnum& vnum, int number = 1);
	RoomID(const String& s);

	// test if RoomID has been initialized correctly
	bool is_valid() const { return !(number_data < 0); }

	// serialize to integer.  leftmost bit indicates validity,
	// next bit always 0, next 14 are number, last 16 are vnum
	int to_int() const {
		if (!is_valid())
			return -1;

		int ret = number_data;
		ret <<= num_bits_vnum();
		return ret | vnum_data;
	}

	const Vnum get_vnum() const;
	const int get_number() const;
	const String to_string(bool short_loc = true) const;

	RoomID(const RoomID& id) : number_data(id.number_data), vnum_data(id.vnum_data) {}
	RoomID& operator=(const RoomID& id) {
		number_data = id.number_data;
		vnum_data = id.vnum_data;
		return *this;
	}

	inline friend bool operator<(const RoomID& lhs, const RoomID& rhs) { return lhs.to_int() < rhs.to_int(); }
	inline friend bool operator==(const RoomID& lhs, const RoomID& rhs) { return lhs.to_int() == rhs.to_int(); }
	inline friend bool operator!=(const RoomID& lhs, const RoomID& rhs) { return !(lhs == rhs); }

	virtual ~RoomID() {}
private:
	short number_data;
	unsigned short vnum_data;

	// leftmost bit of number_data reserved to indicate invalid (negative)
	// next bit is for serialization to int, always 0 to differ from coordinate bit 1
	inline int num_bits_number() const { return 14; }
	inline int num_bits_vnum() const { return 16; }
};

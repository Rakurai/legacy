#include "RoomID.hh"
#include "String.hh"
#include "Format.hh"
#include "Vnum.hh"

RoomID::
RoomID(const Vnum& vnum, int number) : RoomID() {
	if (number < 1 || (number >> num_bits_number()) > 0)
		return; // error

	if (vnum.value() < 1 || (vnum.value() >> num_bits_vnum()) > 0)
		return; // error

	number_data = (short)number-1;
	vnum_data = (unsigned short)vnum.value();
}

RoomID::
RoomID(const String& s) : RoomID() {
	String str(s.strip()); // splitting also strips both returns below

	int num = 1;

	if (str.has_infix(".")) {
		String num_str;
		str = str.lsplit(num_str, ".");

		if (num_str.is_number())
			num = atoi(num_str);
	}

	int vnum = 0;

	if (str.is_number())
		vnum = atoi(str);

	*this = RoomID(Vnum(vnum), num); // will error check
}

const Vnum RoomID::
get_vnum() const {
	if (!is_valid())
		return Vnum(0);

	return Vnum(vnum_data);
}

const int RoomID::
get_number() const {
	if (!is_valid())
		return 0;

	return ((number_data << 2) >> 2)+1;
}

const String RoomID::
to_string(bool short_loc) const {
	if (!is_valid())
		return "(null)";

	String buf;

	Vnum vnum = get_vnum();
	unsigned short num = get_number();

	if (num > 1) {
		if (short_loc)
			buf += Format::format("%d.", num);
		else
			buf += Format::format("%3d.", num);
	}

	if (short_loc)
		buf += Format::format("%d", vnum);
	else
		buf += Format::format("%5d", vnum);

	return buf;
}

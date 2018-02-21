#include "Vnum.hh"
#include "Logging.hh"

unsigned long Vnum::
constrain_value(unsigned long v) {
	if (v >> 63 == 1) {
		Logging::bugf("vnum:  attempt to instantiate with negative value");
		v = 0;
	}

	return v;
}

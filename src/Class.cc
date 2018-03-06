#include "String.hh"
#include "Class.hh"

Class get_class(const String& s) {
	     if (s.is_prefix_of("mage")) return mage;
	else if (s.is_prefix_of("cleric")) return cleric;
	else if (s.is_prefix_of("thief")) return thief;
	else if (s.is_prefix_of("warrior")) return warrior;
	else if (s.is_prefix_of("necromancer")) return necromancer;
	else if (s.is_prefix_of("paladin")) return paladin;
	else if (s.is_prefix_of("bard")) return bard;
	else if (s.is_prefix_of("ranger")) return ranger;

	return none;
}

const String get_class(Class c) {
	switch (c) {
	case mage:	     return "mage";
	case cleric:	 return "cleric";
	case thief:	     return "thief";
	case warrior:	 return "warrior";
	case necromancer:return "necromancer";
	case paladin:	 return "paladin";
	case bard:       return "bard";
	case ranger:     return "ranger";
	default:         return "none";
	}
}


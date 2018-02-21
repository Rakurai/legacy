#include "ExitPrototype.hh"
#include "file.hh"
#include "db.hh"

ExitPrototype::
ExitPrototype(FILE *fp) :
	description(fread_string(fp)),
	keyword(fread_string(fp)) {

	int locks        = fread_number(fp);
	key              = fread_number(fp);
	to_vnum          = fread_number(fp);

	switch (locks) {
	case 1: exit_flags = EX_ISDOOR;                   break;
	case 2: exit_flags = EX_ISDOOR | EX_PICKPROOF;      break;
	case 3: exit_flags = EX_ISDOOR | EX_NOPASS;         break;
	case 4: exit_flags = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF; break;
	}
}

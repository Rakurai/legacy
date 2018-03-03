#include "MobProg.hh"
#include "file.hh"

MobProg::
MobProg(FILE *fp) {
	type = name_to_type(fread_word(fp));

	switch (type) {
	case ERROR_PROG:
		break;

	default:
		arglist = fread_string(fp);
		fread_to_eol(fp);
		comlist = fread_string(fp);
		fread_to_eol(fp);
		break;
	}
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
Flags::Bit MobProg::
name_to_type(const String& name) {
//	if (name == "in_file_prog")   return IN_FILE_PROG;
	if (name == "act_prog")       return ACT_PROG;
	if (name == "speech_prog")    return SPEECH_PROG;
	if (name == "rand_prog")      return RAND_PROG;
	if (name == "rand_area_prog") return RAND_AREA_PROG;
	if (name == "boot_prog")      return BOOT_PROG;
	if (name == "fight_prog")     return FIGHT_PROG;
	if (name == "buy_prog")       return BUY_PROG;
	if (name == "hitprcnt_prog")  return HITPRCNT_PROG;
	if (name == "death_prog")     return DEATH_PROG;
	if (name == "entry_prog")     return ENTRY_PROG;
	if (name == "greet_prog")     return GREET_PROG;
	if (name == "all_greet_prog") return ALL_GREET_PROG;
	if (name == "give_prog")      return GIVE_PROG;
	if (name == "bribe_prog")     return BRIBE_PROG;
	if (name == "tick_prog")      return TICK_PROG;

	return (ERROR_PROG);
}

const String MobProg::
type_to_name(Flags::Bit type) {
	switch (type) {
//	case IN_FILE_PROG:          return "in_file_prog";
	case ACT_PROG:              return "act_prog";
	case SPEECH_PROG:           return "speech_prog";
	case RAND_PROG:             return "rand_prog";
	case RAND_AREA_PROG:        return "rand_area_prog";
	case BOOT_PROG:             return "boot_prog";
	case FIGHT_PROG:            return "fight_prog";
	case BUY_PROG:              return "buy_prog";
	case HITPRCNT_PROG:         return "hitprcnt_prog";
	case DEATH_PROG:            return "death_prog";
	case ENTRY_PROG:            return "entry_prog";
	case GREET_PROG:            return "greet_prog";
	case ALL_GREET_PROG:        return "all_greet_prog";
	case GIVE_PROG:             return "give_prog";
	case BRIBE_PROG:            return "bribe_prog";
	case TICK_PROG:             return "tick_prog";
	default:                    return "ERROR_PROG";
	}
}

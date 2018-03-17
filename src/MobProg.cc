#include "MobProg.hh"
#include "file.hh"
#include "Logging.hh"

MobProg::
MobProg(FILE *fp) {
	type = name_to_type(fread_word(fp));

	switch (type) {
	case ERROR_PROG:
		break;

	default:
		arglist = fread_string(fp);
		fread_to_eol(fp);
		original = fread_string(fp);
		fread_to_eol(fp);
		break;
	}

	if (original.empty())
		return;

	// parse the script into expressions and statements, pre-check for control
	// flow and variable reference errors

	Line::Type last_control;
	String script = original;

	while (!script.empty()) {
		// grab the first token, if it's some mud command we'll put it back
		// lsplit will strip both strings of leading and trailing whitespace
		String word, line;
		script = script.lsplit(word); // split on any whitespace, including newline

		if (word.empty())
			continue;

		Line::Type line_type = Line::get_type(word);

		switch(line_type) {
		case Line::Type::IF:
			last_control = line_type;
			script = script.lsplit(line, "\n"); // take the rest of the line as an expression
			break;

		case Line::Type::OR:
			// "or" can only follow "if" or "or"
			if (last_control != Line::Type::IF
			 && last_control != Line::Type::OR) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = ERROR_PROG;
				return;
			}

			last_control = line_type;
			script = script.lsplit(line, "\n"); // take the rest of the line as an expression
			break;

		case Line::Type::AND:
			// "and" can only follow "if" or "and"
			if (last_control != Line::Type::IF
			 && last_control != Line::Type::AND) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = ERROR_PROG;
				return;
			}

			last_control = line_type;
			script = script.lsplit(line, "\n"); // take the rest of the line as an expression
			break;

		case Line::Type::ELSE:
			// "else" can only follow "if" or "and" or "or"
			if (last_control != Line::Type::IF
			 && last_control != Line::Type::AND
			 && last_control != Line::Type::OR) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = ERROR_PROG;
				return;
			}

			last_control = line_type;
			break;

		case Line::Type::ENDIF:
			last_control = line_type;
			break;

		case Line::Type::BREAK:
			break;

		case Line::Type::COMMAND:
			// take the rest of the line, put it back together with the word
			script = script.lsplit(line, "\n");
			line = word + " " + line;
			break;
		}

		lines.push_back({line_type, line});
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
	if (name == "control_prog")       return CONTROL_PROG;

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
	case CONTROL_PROG:          return "control_prog";
	default:                    return "ERROR_PROG";
	}
}

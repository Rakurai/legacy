#include <map>
#include "MobProg.hh"
#include "file.hh"
#include "Logging.hh"

// possible names for different variables, to build an allowed variable list for each prog type
const String mobv = "iIjkl"; // the mob itself
const String rndv = "rRJKL"; // random char in room
const String mstv = "bBfgh"; // mob's master
const String actv = "nNems"; // the actor
const String dobv = "oOa";   // direct object
const String iobv = "pPA";   // indirect object (act progs only)
const String vicv = "tTEMS"; // victim (act progs only)

const std::map<MobProg::Type, mobprog_data_t> mobprog_data = {
	{ MobProg::Type::ACT_PROG,       { "act_prog",       mobv + mstv + rndv + actv + dobv + iobv + vicv }},
	{ MobProg::Type::SPEECH_PROG,    { "speech_prog",    mobv + mstv + rndv + actv}},
	{ MobProg::Type::RAND_PROG,      { "rand_prog",      mobv + mstv + rndv }},
	{ MobProg::Type::FIGHT_PROG,     { "fight_prog",     mobv + mstv + rndv + actv }},
	{ MobProg::Type::DEATH_PROG,     { "death_prog",     mobv + mstv + rndv }},
	{ MobProg::Type::HITPRCNT_PROG,  { "hitprcnt_prog",  mobv + mstv + rndv + actv }},
	{ MobProg::Type::ENTRY_PROG,     { "entry_prog",     mobv + mstv + rndv }},
	{ MobProg::Type::GREET_PROG,     { "greet_prog",     mobv + mstv + rndv + actv }},
	{ MobProg::Type::ALL_GREET_PROG, { "all_greet_prog", mobv + mstv + rndv + actv }},
	{ MobProg::Type::GIVE_PROG,      { "give_prog",      mobv + mstv + rndv + actv + dobv }},
	{ MobProg::Type::BRIBE_PROG,     { "bribe_prog",     mobv + mstv + rndv + actv + dobv }},
	{ MobProg::Type::BUY_PROG,       { "buy_prog",       mobv + mstv + rndv + actv }},
	{ MobProg::Type::TICK_PROG,      { "tick_prog",      mobv + mstv + rndv }},
	{ MobProg::Type::BOOT_PROG,      { "boot_prog",      mobv + mstv + rndv }},
	{ MobProg::Type::RAND_AREA_PROG, { "rand_area_prog", mobv + mstv + rndv }},
	{ MobProg::Type::CONTROL_PROG,   { "control_prog",   mobv + mstv + rndv + actv }},
};

MobProg::
MobProg(FILE *fp, Vnum vnum) {
	String name = fread_word(fp);
	type = name_to_type(name);

	switch (type) {
	case Type::ERROR_PROG:
		break;

	default:
		arglist = fread_string(fp);
		fread_to_eol(fp);
		original = fread_string(fp);
		fread_to_eol(fp);
		break;
	}

	if (type == Type::ERROR_PROG || original.empty())
		return;

	// parse the script into expressions and statements, pre-check for control
	// flow and variable reference errors

	Line::Type last_control, last_type;
	String script = original;
	const String& allowed_vars = mobprog_data.find(type)->second.allowed_vars;
	int unclosed_ifs = 0;

	while (!script.empty()) {
		String word, line;

		// grab the first token, if it's some mud command we'll put it back
		// lsplit will left strip both strings of leading whitespace
		// parse into the first word and the expression
		script = script.lsplit(line, "\n");
		line = line.strip(" \t\r").lsplit(word, " \t");

		if (word.empty())
			continue;

		Line::Type line_type = Line::get_type(word);

		switch(line_type) {
		case Line::Type::IF:
			last_control = line_type;
			unclosed_ifs++;
			break;

		case Line::Type::OR:
			// "or" can only follow "if" or "or", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::OR) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = Type::ERROR_PROG;
				return;
			}

			last_control = line_type;
			break;

		case Line::Type::AND:
			// "and" can only follow "if" or "and", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::AND) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = Type::ERROR_PROG;
				return;
			}

			last_control = line_type;
			break;

		case Line::Type::ELSE:
			// "else" can only follow "if" or "and" or "or" controls
			if (last_control != Line::Type::IF
			 && last_control != Line::Type::AND
			 && last_control != Line::Type::OR) {
				Logging::bugf("MobProg: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = Type::ERROR_PROG;
				return;
			}

			// "else" must have statements or a nested control structure in the block
			if (last_type != Line::Type::COMMAND
			 && last_type != Line::Type::BREAK
			 && last_type != Line::Type::ENDIF) {
				Logging::bugf("MobProg: 'else' has no statements to follow");
				this->type = Type::ERROR_PROG;
				return;
			}

			// special case for 'else' followed by 'if' on the same line, put it back
			if (line.has_prefix("if ")) {
				script = line + "\n" + script;
				line = "";
			}

			last_control = line_type;
			break;

		case Line::Type::ENDIF:
			// "endif" must have statements or a nested control structure in the block
			if (last_type != Line::Type::COMMAND
			 && last_type != Line::Type::BREAK
			 && last_type != Line::Type::ENDIF) {
				Logging::bugf("MobProg: 'endif' has no statements to follow");
				this->type = Type::ERROR_PROG;
				return;
			}

			last_control = line_type;
			unclosed_ifs--;
			break;

		case Line::Type::BREAK:
			break;

		case Line::Type::COMMAND:
			// put the line back together with the word
			line = word + " " + line;
			break;
		}

		last_type = line_type;

		// test the line for variable usage that doesn't make sense
		std::size_t pos = 0;

		while ((pos = line.find("$", pos)) != std::string::npos && pos < line.length()-1) {
			char letter = line[pos+1];
			pos += 2;

			if (letter == '$')
				continue; // skip a double $$

			if (strchr(allowed_vars, letter) == nullptr) {
				Logging::bugf("MobProg: variable $%c is undefined for prog type '%s'", letter, name);
				this->type = Type::ERROR_PROG;
				return;
			}
		}

		lines.push_back({line_type, line});
	}

	if (unclosed_ifs > 0) {
		Logging::bugf("MobProg: warning: %s on mob %d has unclosed if statements, potentially ambiguous execution",
			name, vnum);
	}
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
MobProg::Type MobProg::
name_to_type(const String& name) {
	for (const auto& pair : mobprog_data)
		if (pair.second.name == name)
			return pair.first;

	return Type::ERROR_PROG;
}

const String MobProg::
type_to_name(MobProg::Type type) {
	const auto pair = mobprog_data.find(type);

	if (pair == mobprog_data.cend())
		return "ERROR_PROG";

	return pair->second.name;
}

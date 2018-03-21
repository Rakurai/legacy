#include <map>
#include <stack>
#include "progs/Prog.hh"
#include "progs/Context.hh"
#include "progs/Operator.hh"
#include "progs/Symbol.hh"
#include "file.hh"
#include "Logging.hh"

namespace progs {

struct prog_data_t {
	const String name;
	const String allowed_vars;
};

// possible names for different variables, to build an allowed variable list for each prog type
const String mobv = "iIjkl"; // the mob itself
const String rndv = "rRJKL"; // random char in room
const String mstv = "bBfgh"; // mob's master
const String actv = "nNems"; // the actor
const String dobv = "oOa";   // direct object
const String iobv = "pPA";   // indirect object (act progs only)
const String vicv = "tTEMS"; // victim (act progs only)

const std::map<Prog::Type, prog_data_t> prog_data = {
	{ Prog::Type::ACT_PROG,       { "act_prog",       mobv + mstv + rndv + actv + dobv + iobv + vicv }},
	{ Prog::Type::SPEECH_PROG,    { "speech_prog",    mobv + mstv + rndv + actv}},
	{ Prog::Type::RAND_PROG,      { "rand_prog",      mobv + mstv + rndv }},
	{ Prog::Type::FIGHT_PROG,     { "fight_prog",     mobv + mstv + rndv + actv }},
	{ Prog::Type::DEATH_PROG,     { "death_prog",     mobv + mstv + rndv }},
	{ Prog::Type::HITPRCNT_PROG,  { "hitprcnt_prog",  mobv + mstv + rndv + actv }},
	{ Prog::Type::ENTRY_PROG,     { "entry_prog",     mobv + mstv + rndv }},
	{ Prog::Type::GREET_PROG,     { "greet_prog",     mobv + mstv + rndv + actv }},
	{ Prog::Type::ALL_GREET_PROG, { "all_greet_prog", mobv + mstv + rndv + actv }},
	{ Prog::Type::GIVE_PROG,      { "give_prog",      mobv + mstv + rndv + actv + dobv }},
	{ Prog::Type::BRIBE_PROG,     { "bribe_prog",     mobv + mstv + rndv + actv + dobv }},
	{ Prog::Type::BUY_PROG,       { "buy_prog",       mobv + mstv + rndv + actv }},
	{ Prog::Type::TICK_PROG,      { "tick_prog",      mobv + mstv + rndv }},
	{ Prog::Type::BOOT_PROG,      { "boot_prog",      mobv + mstv + rndv }},
	{ Prog::Type::RAND_AREA_PROG, { "rand_area_prog", mobv + mstv + rndv }},
	{ Prog::Type::CONTROL_PROG,   { "control_prog",   mobv + mstv + rndv + actv }},
};

Prog::
Prog(FILE *fp, Vnum vnum) {
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
	const String& allowed_vars = prog_data.find(type)->second.allowed_vars;
	int unclosed_ifs = 0;

	while (!script.empty()) {
		String word, line;

		// grab the first token, if it's some mud command we'll put it back
		// lsplit will left strip both strings of leading whitespace
		// parse into the first word and the expression
		script = script.lsplit(line, "\n");
		line = line.strip(" \t\r");

		// allow for commenting out a line
		if (line[0] == '*')
			continue;

		line = line.lsplit(word, " \t");

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
				Logging::bugf("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = Type::ERROR_PROG;
				return;
			}

			last_control = line_type;
			break;

		case Line::Type::AND:
			// "and" can only follow "if" or "and", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::AND) {
				Logging::bugf("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
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
				Logging::bugf("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));
				this->type = Type::ERROR_PROG;
				return;
			}

			// "else" must have statements or a nested control structure in the block
			if (last_type != Line::Type::COMMAND
			 && last_type != Line::Type::BREAK
			 && last_type != Line::Type::ENDIF) {
				Logging::bugf("progs::Prog: 'else' has no statements to follow");
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
				Logging::bugf("progs::Prog: 'endif' has no statements to follow");
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
				Logging::bugf("progs::Prog: variable $%c is undefined for prog type '%s'", letter, name);
				this->type = Type::ERROR_PROG;
				return;
			}
		}

		lines.push_back({line_type, line});
	}

	if (unclosed_ifs > 0) {
		Logging::bugf("progs::Prog: warning: %s on vnum %d has unclosed if statements, potentially ambiguous execution",
			name, vnum);
	}
	if (unclosed_ifs < 0) {
		Logging::bugf("progs::Prog: warning: %s on vnum %d has extra endif statements, potentially missing expressions",
			name, vnum);
	}
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
Prog::Type Prog::
name_to_type(const String& name) {
	for (const auto& pair : prog_data)
		if (pair.second.name == name)
			return pair.first;

	return Type::ERROR_PROG;
}

const String Prog::
type_to_name(Prog::Type type) {
	const auto pair = prog_data.find(type);

	if (pair == prog_data.cend())
		return "ERROR_PROG";

	return pair->second.name;
}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
bool evaluate_expression(String expression, Context& context) {
	// get rid of unneeded spaces
	expression = expression.strip();

	if (expression.empty())
		throw Format::format("progs::evaluate_expression: empty expression");

	bool invert = false;

	if (expression[0] == '!') {
		invert = true;
		expression.erase(0, 1);
	}

	// search for a binary operator in the string.  if none found,
	// treat the whole expression as a symbol and evaluate against == 1
	for (int i = Operator::Type::first; i < Operator::Type::size; i++) {
		Operator opr((Operator::Type)i);
		std::size_t pos = expression.find(opr.to_string());

		if (pos == std::string::npos)
			continue;

		if (expression.length() - pos - opr.to_string().length() <= 0)
			throw Format::format("progs::evaluate_expression: empty expression after operator", opr.to_string());

		if (invert)
			throw Format::format("progs::evaluate_expression: unary '!' encountered in binary expression");

		String lhs = expression.substr(0, pos);
		String rhs = expression.substr(pos + opr.to_string().length());

		return Operator((Operator::Type)i).evaluate(
			Symbol::parse(lhs, "")->evaluate(context),
			Symbol::parse(rhs, "")->evaluate(context)
		);
	}

	return Operator(Operator::Type::is_equal_to).evaluate(
		Symbol::parse(expression, "")->evaluate(context),
		invert ? "0" : "1"
	);
}

/* I didn't like the complex recursive function that was part of the original mobprog code, so
 * I rewrote it.  Run of the mill stack processor.  On encountering an 'if', we push the evaluation T/F onto
 * the stack.  On encountering commands, we look at the latest entry to decide if we're currently
 * executing those commands.  On encountering another 'if', we push its evaluation unless the parent
 * is false, in which case we push false.  On encountering 'else', we pop and push the opposite,
 * but again, if the parent is false we push false.  On encountering 'endif', we pop. -- Montrey */
void Prog::
execute(Context& context) const {
try {
	std::stack<bool> stack;
	bool opposite;

	for (const Line& line : lines) {
		switch (line.type) {
		case Line::Type::IF:
			// if we're under a false evaluation, don't even bother evaluating the line, we'll stay false
			if (!stack.empty() && stack.top() == false) {
				stack.push(false);
				continue;
			}

			// otherwise push the evaluation
			stack.push(evaluate_expression(line.text, context));
			break;

		case Line::Type::OR:
			if (stack.empty())
				throw String("progs::execute: 'or' encountered without 'if'");

			// "or" won't change a true evaluation, don't bother evaluating
			if (stack.top() == true)
				continue;

			// "or" can change a false evaluation only if parent is not false, peek at the parent
			stack.pop();

			if (!stack.empty() && stack.top() == false) {
				// put the false parent back
				stack.push(false);
				continue;
			}

			// otherwise, the false parent is popped, we'll see if it changes
			stack.push(evaluate_expression(line.text, context));
			break;

		case Line::Type::AND:
			if (stack.empty())
				throw String("progs::execute: 'and' encountered without 'if'");

			// "and" won't change a false evaluation, don't bother evaluating
			if (stack.top() == false)
				continue;

			// otherwise, pop the true parent and we'll see if it changes
			stack.pop();

			stack.push(evaluate_expression(line.text, context));
			break;

		case Line::Type::ELSE:
			if (stack.empty())
				throw String("progs::execute: 'else' encountered without 'if'");

			// switch evaluations now, unless the parent is false
			opposite = !stack.top();
			stack.pop();

			if (!stack.empty() && stack.top() == false)
				stack.push(false);
			else
				stack.push(opposite);

			break;

		case Line::Type::ENDIF:
			if (stack.empty())
				throw String("progs::execute: 'endif' encountered without 'if'");

			stack.pop();
			break;

		case Line::Type::BREAK:
			if (!stack.empty() && stack.top() == false)
				continue;

			// special command, get out immediately
			return;

		case Line::Type::COMMAND:
			if (!stack.empty() && stack.top() == false)
				continue;

			// mud commands
			context.process_command(line.text);

			// did the mob extract itself?
			if (context.self_is_garbage())
				return;
		}
	}

	if (!stack.empty())
		throw String("progs::execute: reached end of script without closing 'if' statements");
} catch (String e) {
	Logging::bugf("Exception caught in mobprog on mob %d:", context.vnum());
	Logging::bugf(e);
}
}

} // namespace progs


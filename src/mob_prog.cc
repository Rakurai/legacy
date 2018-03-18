/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include <cstring>
#include <stack>

#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Flags.hh"
#include "GameTime.hh"
#include "Logging.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "MobProg.hh"
#include "MobProgActList.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"
#include "find.hh"
#include "Game.hh"
#include "progs/Operator.hh"

bool MOBtrigger;

/*
 * Local function prototypes
 */

String  mprog_translate         args((char ch, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo, Character *rndm));
void    mprog_process_cmnd      args((const String& cmnd, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo, Character *rndm));
void    mprog_driver            args((const MobProg *mprg, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo));

/***************************************************************************
 * Local function code and brief comments.
 */

// tired of all this duplicate code
Character *get_char_target(const String& var, Character *mob, Character *actor, Character *vict, Character *rndm) {
	if (var.length() == 2) {
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'i': return mob;
			case 'b': return mob->master;
			case 'n': return actor;
			case 't': return vict;
			case 'r': return rndm;
		}
	}

	throw Format::format("progs::get_char_target: bad target '%s'", var);
}

Object *get_obj_target(const String& var, Object *obj, Object *v_obj) {
	if (var.length() == 2) {
		switch (var[1]) {/* arg should be "$*" so just get the letter */
			case 'o': return obj;
			case 'p': return v_obj;
		}
	}

	throw Format::format("progs::get_obj_target: bad target '%s'", var);
}

const String dereference_variable(const String& var, Character *mob, Character *actor, Object *obj, void *vo, Character *rndm) {
	// get the target
	String target_name, member_name;
	member_name = var.lsplit(target_name, ".").strip();
	target_name = target_name.rstrip();

	if (member_name.empty())
		member_name = "name";

	// try it as a character
	try {
		Character *target = get_char_target(target_name, mob, actor, (Character*)vo, rndm);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", target_name);

		if (member_name == "name") return target->name;

		throw Format::format("progs::dereference_variable: unknown variable member '%s'", member_name);
	}
	catch (String e) {
		throw;
	}

	// try it as an obj
	try {
		Object *target = get_obj_target(target_name, obj, (Object*)vo);

		if (target == nullptr)
			throw Format::format("progs::dereference_variable: variable '%s' is null", target_name);

		if (member_name == "name") return target->short_descr;

		throw Format::format("progs::dereference_variable: unknown variable member '%s'", member_name);
	}
	catch (String e) {
		throw;
	}
}

const String parse_variable(String& str) {
	// split on whitespace or on snuggled operator
	if (str.length() < 2 || str[0] != '$')
		throw Format::format("progs::parse_variable: unable to parse variable from '%s'", str);

	String var = str.substr(0, 2);
	str.erase(0, 2);

	// accesses a member?
	if (str[0] == '.') {
		if (str.length() < 2)
			throw Format::format("progs::parse_variable: unable to parse variable member from '%s'", str);

		var += '.';
		str.erase(0, 1);

		while(true) {
			if (str.length() == 0)
				break;

			if (!isalpha(str[0])
			 && !isdigit(str[0])
			 && str[0] != '_')
				break;

			var += str[0];
			str.erase(0, 1);
		}
	}

	return var;
}

const String compute_function(const String& fn, String args, Character *mob, Character *actor, Object *obj, void *vo, Character *rndm) {
	Character *vict = (Character *)vo;
	Object *v_obj = (Object *)vo;

	if (!strcmp(fn, "rand"))
		return (number_percent() <= atoi(args)) ? "1" : "0";

	if (!strcmp(fn, "mudtime"))
		return Format::format("%d", Game::world().time.hour);

	if (!strcmp(fn, "get_state")) {
		String arg1, arg2;
		arg2 = String(args).lsplit(arg1, ",");

		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::compute_function: bad argument '%s' to '%s'", args, fn);

		Character *target;
		if ((target = get_char_target(arg1, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		const auto entry = target->mpstate.find(arg2);

		int state = 0;

		if (entry != target->mpstate.cend())
			state = entry->second;

		return Format::format("%d", state);
	}

	if (!strcmp(fn, "iscarrying")) {
		String arg1, arg2;
		arg2 = String(args).lsplit(arg1, ",");

		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::compute_function: bad argument '%s' to '%s'", args, fn);

		Character *target;
		if ((target = get_char_target(arg1, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return (get_obj_carry(target, arg2) == nullptr) ? "0" : "1";
	}

	if (!strcmp(fn, "iswearing")) {
		String arg1, arg2;
		arg2 = String(args).lsplit(arg1, ",");

		if (arg1.empty() || arg2.empty())
			throw Format::format("progs::compute_function: bad argument '%s' to '%s'", args, fn);

		Character *target;
		if ((target = get_char_target(arg1, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return (get_obj_wear(target, arg2) == nullptr) ? "0" : "1";
	}

	if (!strcmp(fn, "ispc")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return !target->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isnpc")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return target->is_npc() ? "1" : "0";
	}

	if (!strcmp(fn, "isgood")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_GOOD(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isevil")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_EVIL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isneutral")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_NEUTRAL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isfight")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return target->fighting ? "1" : "0";
	}

	if (!strcmp(fn, "isimmort")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_IMMORTAL(target) ? "1" : "0";
	}

	if (!strcmp(fn, "iskiller")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_KILLER(target) ? "1" : "0";
	}

	if (!strcmp(fn, "isthief")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return IS_THIEF(target) ? "1" : "0";
	}

	if (!strcmp(fn, "ischarmed")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return affect::exists_on_char(target, affect::type::charm_person) ? "1" : "0";
	}

	if (!strcmp(fn, "isfollow")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return (target->master && target->master->in_room == target->in_room) ? "1" : "0";
	}

	if (!strcmp(fn, "ismaster")) { /* is $ their master? */
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return mob->master == target ? "1" : "0";
	}

	if (!strcmp(fn, "isleader")) { /* is $ their leader? */
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return mob->leader == target ? "1" : "0";
	}
	if (!strcmp(fn, "hitprcnt")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->hit / GET_MAX_HIT(target));
	}

	if (!strcmp(fn, "inroom")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return target->in_room->location.to_string();
	}

	if (!strcmp(fn, "sex")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", GET_ATTR_SEX(target));
	}

	if (!strcmp(fn, "position")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->position);
	}

	if (!strcmp(fn, "level")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->level);
	}

	if (!strcmp(fn, "class")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->guild);
	}

	if (!strcmp(fn, "goldamt")) {
		Character *target;
		if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->gold);
	}

	if (!strcmp(fn, "objtype")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->item_type);
	}

	if (!strcmp(fn, "objval0")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->value[0]);
	}

	if (!strcmp(fn, "objval1")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->value[1]);
	}

	if (!strcmp(fn, "objval2")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->value[2]);
	}

	if (!strcmp(fn, "objval3")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->value[3]);
	}

	if (!strcmp(fn, "objval4")) {
		Object *target;
		if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
			throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

		return Format::format("%d", target->value[4]);
	}

	if (!strcmp(fn, "number")) {
		// try it as a character
		try {
			Character *target;
			if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

			return Format::format("%d", target->pIndexData->vnum);
		}
		// try it as an obj
		catch (String e) {
			Object *target;
			if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

			return Format::format("%d", target->pIndexData->vnum);
		}
	}

	if (!strcmp(fn, "name")) {
		// try it as a character
		try {
			Character *target;
			if ((target = get_char_target(args, mob, actor, vict, rndm)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

			return target->name;
		}
		// try it as an obj
		catch (String e) {
			Object *target;
			if ((target = get_obj_target(args, obj, v_obj)) == nullptr)
				throw Format::format("progs::compute_function: null target '%s' to '%s'", args, fn);

			return target->name;
		}
	}

	throw Format::format("progs::compute_function: unknown function name '%s'", fn);
}

const String parse_function(String& str, String& args) {
	/* get whatever comes before the left paren.. ignore spaces */
	String fn, buf;
	String remainder = str.lsplit(fn, "(").lstrip();
	fn = fn.rstrip();

	if (fn.empty() || remainder.empty())
		throw Format::format("progs::parse_function: unable to find function name in '%s'", str);

	/* get whatever is in between the parens.. ignore spaces */
	remainder = remainder.lsplit(buf, ")").strip();
	buf = buf.strip();

	if (buf.empty())
		throw Format::format("progs::parse_function: unable to find arguments in '%s'", str);

	str.assign(remainder.lstrip());
	args.assign(buf);
	return fn;
}

// take an operand from a string, dereference or compute the function, return the result
const String parse_operand(String& str, Character *mob, Character *actor, Object *obj, void *vo, Character *rndm) {
	str.assign(str.lstrip());

	// if this is just a variable, dereference and return
	if (str[0] == '$') {
		String var = parse_variable(str);
		return dereference_variable(var, mob, actor, obj, vo, rndm);
	}

	// otherwise interpret as a function(args)
	String args;
	String fn = parse_function(str, args);
	return compute_function(fn, args, mob, actor, obj, vo, rndm);
}

// take an operator from the string, be careful of snuggling with rhs operand
const Operator parse_operator(String& str) {
	str.assign(str.lstrip());

	if (str.empty())
		throw Format::format("progs::parse_operator: empty string to parse");

	int take_chars = 0;

	if (str.has_prefix("==")
	 || str.has_prefix("!=")
	 || str.has_prefix("!/")
	 || str.has_prefix("<=")
	 || str.has_prefix(">="))
	 	take_chars = 2;
	else if (
		str[0] == '/'
	 || str[0] == '>'
	 || str[0] == '<'
	 || str[0] == '&'
	 || str[0] == '|')
		take_chars = 1;

	if (take_chars == 0)
		throw Format::format("progs::parse_operator: no valid operator found in '%s'", str);

	Operator opr = str_to_operator(str.substr(0, take_chars));

	if (opr == Operator::error)
		throw Format::format("progs::parse_operator: no valid operator found in '%s'", str);

	str.erase(0, take_chars);
	return opr;
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
bool evaluate_expression(String expression, Character *mob, Character *actor,
                     Object *obj, void *vo, Character *rndm)
{
	// get rid of unneeded spaces
	expression = expression.strip();

	if (expression.empty())
		throw Format::format("progs::evaluate_expression: empty expression");

	bool invert = false;

	if (expression[0] == '!') {
		invert = true;
		expression.erase(0, 1);
	}

	const String lhs = parse_operand(expression, mob, actor, obj, vo, rndm);

	// defaults, in case there is no operator and rhs
	Operator opr = Operator::is_equal_to;
	String rhs = invert ? "0" : "1";

	if (!expression.empty()) {
		if (invert)
			throw Format::format("progs::evaluate_expression: unary '!' encountered in binary expression");

		opr = parse_operator(expression);
		rhs = parse_operand(expression, mob, actor, obj, vo, rndm);
	}

	return evaluate(lhs, opr, rhs);
}

/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
/* Yesterday, this routine *did* crash on a syntactically correct
 * mobprog. com_list was nullptr, and yet its value was being checked.
 * While the code is terriffic in general, I think the original author
 * was a bit fuzzy on the difference between nullptr and an empty string.
 * Possibly all the return '\0's should be changed to return "".
 * -- Elrac
 */

/* I didn't like the complex recursive function that was part of the original mobprog code, so
 * I rewrote it.  Run of the mill stack processor.  On encountering an 'if', we push the evaluation T/F onto
 * the stack.  On encountering commands, we look at the latest entry to decide if we're currently
 * executing those commands.  On encountering another 'if', we push its evaluation unless the parent
 * is false, in which case we push false.  On encountering 'else', we pop and push the opposite,
 * but again, if the parent is false we push false.  On encountering 'endif', we pop. -- Montrey */
void mprog_process(const MobProg *mprg, Character *mob, Character *actor, Object *obj, void *vo, Character *rndm) {
	std::stack<bool> stack;
	bool opposite;

	for (const MobProg::Line& line : mprg->lines) {
		switch (line.type) {
		case MobProg::Line::Type::IF:
			// if we're under a false evaluation, don't even bother evaluating the line, we'll stay false
			if (!stack.empty() && stack.top() == false) {
				stack.push(false);
				continue;
			}

			// otherwise push the evaluation
			switch(evaluate_expression(line.text, mob, actor, obj, vo, rndm)) {
				case 1: stack.push(true); break;
				case 0: stack.push(false); break;
				default: return;
			}

			break;
		case MobProg::Line::Type::OR:
			if (stack.empty()) {
				Logging::bugf("mprog_process: 'or' encountered without 'if', mob %d", mob->pIndexData->vnum);
				return;
			}

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
			switch(evaluate_expression(line.text, mob, actor, obj, vo, rndm)) {
				case 1: stack.push(true); break;
				case 0: stack.push(false); break;
				default: return;
			}

			break;
		case MobProg::Line::Type::AND:
			if (stack.empty()) {
				Logging::bugf("mprog_process: 'and' encountered without 'if', mob %d", mob->pIndexData->vnum);
				return;
			}

			// "and" won't change a false evaluation, don't bother evaluating
			if (stack.top() == false)
				continue;

			// otherwise, pop the true parent and we'll see if it changes
			stack.pop();

			switch(evaluate_expression(line.text, mob, actor, obj, vo, rndm)) {
				case 1: stack.push(true); break;
				case 0: stack.push(false); break;
				default: return;
			}

			break;
		case MobProg::Line::Type::ELSE:
			if (stack.empty()) {
				Logging::bugf("mprog_process: 'else' encountered without 'if', mob %d", mob->pIndexData->vnum);
				return;
			}

			// switch evaluations now, unless the parent is false
			opposite = !stack.top();
			stack.pop();

			if (!stack.empty() && stack.top() == false)
				stack.push(false);
			else
				stack.push(opposite);

			break;
		case MobProg::Line::Type::ENDIF:
			if (stack.empty()) {
				Logging::bugf("mprog_process: 'endif' encountered without 'if', mob %d", mob->pIndexData->vnum);
				return;
			}

			stack.pop();
			break;

		case MobProg::Line::Type::BREAK:
			if (!stack.empty() && stack.top() == false)
				continue;

			// special command, get out immediately
			return;

		case MobProg::Line::Type::COMMAND:
			if (!stack.empty() && stack.top() == false)
				continue;

			// mud commands
			mprog_process_cmnd(line.text, mob, actor, obj, vo, rndm);

			if (mob->is_garbage()) // purged themselves or died or something
				return;
		}
	}

	if (!stack.empty())
		Logging::bugf("mprog_process: reached end of script without closing 'if' statements, mob %d", mob->pIndexData->vnum);
}


/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
String mprog_translate(char ch, Character *mob, Character *actor,
                     Object *obj, void *vo, Character *rndm)
{
	static char *he_she        [] = { "it",  "he",  "she" };
	static char *him_her       [] = { "it",  "him", "her" };
	static char *his_her       [] = { "its", "his", "her" };
	Character   *vict             = (Character *) vo;
	Object    *v_obj            = (Object *) vo;
	String t;

	switch (ch) {
	case 'i':
		t = mob->name.lsplit();
		break;

	case 'I':
		t = mob->short_descr;
		break;

	case 'n':
		if (actor) {
			if (can_see_char(mob, actor))
				t = actor->name;

			if (!actor->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'N':
		if (actor) {
			if (can_see_char(mob, actor)) {
				if (actor->is_npc())
					t = actor->short_descr;
				else {
					t = actor->name;
					t += " ";
					t += actor->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'b':
		if (mob->master) {
			if (can_see_char(mob, mob->master))
				t = mob->master->name;

			if (!mob->master->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'B':
		if (mob->master) {
			if (can_see_char(mob, mob->master)) {
				if (mob->master->is_npc())
					t = mob->master->short_descr;
				else {
					t = mob->master->name;
					t += " ";
					t += mob->master->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 't':
		if (vict) {
			if (can_see_char(mob, vict))
				t = vict->name;

			if (!vict->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'T':
		if (vict) {
			if (can_see_char(mob, vict)) {
				if (vict->is_npc())
					t = vict->short_descr;
				else {
					t = vict->name;
					t += " ";
					t += vict->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'r':
		if (rndm) {
			if (can_see_char(mob, rndm))
				t = rndm->name.lsplit();
			else
				t = "someone";

			if (!rndm->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'R':
		if (rndm) {
			if (can_see_char(mob, rndm)) {
				if (rndm->is_npc())
					t = rndm->short_descr;
				else {
					t = rndm->name;
					t += " ";
					t += rndm->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'e':
		if (actor)
			t = can_see_char(mob, actor) ? he_she[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 'm':
		if (actor)
			t = can_see_char(mob, actor) ? him_her[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 's':
		if (actor)
			t = can_see_char(mob, actor) ? his_her[GET_ATTR_SEX(actor)] : "someone's";

		break;

	case 'E':
		if (vict)
			t = can_see_char(mob, vict) ? he_she[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'M':
		if (vict)
			t = can_see_char(mob, vict) ? him_her[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'S':
		if (vict)
			t = can_see_char(mob, vict) ? his_her[GET_ATTR_SEX(vict)] : "someone's";

		break;

	case 'f':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? he_she[GET_ATTR_SEX(mob->master)] : "someone";

		break;

	case 'g':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? him_her[GET_ATTR_SEX(mob->master)] : "someone";

		break;

	case 'h':
		if (mob->master)
			t = can_see_char(mob, mob->master) ? his_her[GET_ATTR_SEX(mob->master)] : "someone's";

		break;

	case 'j':
		t = he_she[GET_ATTR_SEX(mob)];
		break;

	case 'k':
		t = him_her[GET_ATTR_SEX(mob)];
		break;

	case 'l':
		t = his_her[GET_ATTR_SEX(mob)];
		break;

	case 'J':
		if (rndm)
			t = can_see_char(mob, rndm) ? he_she[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'K':
		if (rndm)
			t = can_see_char(mob, rndm) ? him_her[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'L':
		if (rndm)
			t = can_see_char(mob, rndm) ? his_her[GET_ATTR_SEX(rndm)] : "someone's";

		break;

	case 'o':
		if (obj)
			t = can_see_obj(mob, obj) ? String(obj->name).lsplit() : "something";

		break;

	case 'O':
		if (obj)
			t = can_see_obj(mob, obj) ? obj->short_descr : "something";

		break;

	case 'p':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? String(v_obj->name).lsplit() : "something";

		break;

	case 'P':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? v_obj->short_descr : "something";

		break;

	case 'a':
		if (obj)
			switch (tolower(obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case 'A':
		if (v_obj)
			switch (tolower(v_obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case '$':
		t = "$";
		break;

	default:
		Logging::bugf("Mob: %d bad $var", mob->pIndexData->vnum);
		break;
	}

	return t;
}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd(const String& cmnd, Character *mob, Character *actor,
                        Object *obj, void *vo, Character *rndm)
{
	String buf;
	const char *str = cmnd.c_str();

	while (*str != '\0') {
		if (*str != '$') {
			buf += *str++;
			continue;
		}

		str++;
		buf += mprog_translate(*str, mob, actor, obj, vo, rndm);
		str++;
	}

	interpret(mob, buf);
	return;
}

/* The main focus of the MOBprograms.  This routine is called
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver(const MobProg *mprg, Character *mob, Character *actor,
                  Object *obj, void *vo)
{
	Character *rndm  = nullptr;
	int        count = 0;

	/*    if affect::exists_on_char( mob, affect::type::charm_person )
	        return;                                 why? :P  -- Montrey */

	/* get a random visable mortal player who is in the room with the mob */
	for (Character *vch = mob->in_room->people; vch; vch = vch->next_in_room) {
		if (!vch->is_npc()
		    &&  !IS_IMMORTAL(vch)
		    &&  can_see_char(mob, vch)) {
			if (number_range(0, count) == 0)
				rndm = vch;

			count++;
		}
	}

	try {
		mprog_process(mprg, mob, actor, obj, vo, rndm);
	}
	catch (String e) {
		Logging::bugf("Exception caught in mobprog on mob %d:", mob->pIndexData->vnum);
		Logging::bugf(e);
	}
} /* end mprog_driver() */

/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check(const String& arg, Character *mob, Character *actor,
                          Object *obj, void *vo, MobProg::Type type)
{
	char        temp1[ MAX_STRING_LENGTH ];
	char        temp2[ MAX_INPUT_LENGTH ];
	const char       *list;
	char       *start;
	char       *dupl;
	char       *end;

	String word;

	for (const auto mprg : mob->pIndexData->mobprogs) {
		if (mob->is_garbage())
			break;

		if (mprg->type == type) {
			strcpy(temp1, mprg->arglist);

			for (unsigned int i = 0; i < strlen(temp1); i++)
				temp1[i] = tolower(temp1[i]);

			list = temp1;
			strcpy(temp2, arg);
			dupl = temp2;

			for (unsigned int i = 0; i < strlen(dupl); i++)
				dupl[i] = tolower(dupl[i]);

			if ((list[0] == 'p') && (list[1] == ' ')) {
				list += 2;

				while ((start = std::strstr(dupl, list)))
					if ((start == dupl || *(start - 1) == ' ')
					    && (*(end = start + strlen(list)) == ' '
					        || *end == '\n'
					        || *end == '\r'
					        || *end == '\0')) {
						mprog_driver(mprg, mob, actor, obj, vo);
						break;
					}
					else
						dupl = start + 1;
			}
			else {
				list = one_argument(list, word);

				for (; !word.empty(); list = one_argument(list, word))
					while ((start = strstr(dupl, word)))
						if ((start == dupl || *(start - 1) == ' ')
						    && (*(end = start + strlen(word)) == ' '
						        || *end == '\n'
						        || *end == '\r'
						        || *end == '\0')) {
							mprog_driver(mprg, mob, actor, obj, vo);
							break;
						}
						else
							dupl = start + 1;
			}
		}
	}

	return;
}

bool mprog_percent_check(Character *mob, Character *actor, Object *obj,
                         void *vo, MobProg::Type type)
{
	for (const auto mprg : mob->pIndexData->mobprogs) {
		if (mob->is_garbage())
			break;

		if ((mprg->type == type)
		    && (number_percent() < atoi(mprg->arglist))) {
			mprog_driver(mprg, mob, actor, obj, vo);

			if (type != MobProg::Type::GREET_PROG && type != MobProg::Type::ALL_GREET_PROG)
				return true;
		}
	}

	return false;
}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger(const char *buf, Character *mob, Character *ch,
                       Object *obj, void *vo)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::ACT_PROG))) {
		MobProgActList *tmp_act = new MobProgActList;

		tmp_act->next	= mob->mpact;
		mob->mpact      = tmp_act;
		mob->mpact->buf = buf;
		mob->mpact->ch  = ch;
		mob->mpact->obj = obj;
		mob->mpact->vo  = vo;
	}

	return;
}

void mprog_bribe_trigger(Character *mob, Character *ch, int amount)
{
	Object *obj;

	if (!mob->is_npc())
		return;

	// this object is used for messages in the prog, it doesn't actually go to the mob,
	// they already have the cash
	if ((obj = create_money(0, amount)) == nullptr)
		return;

	for (const auto mprg : mob->pIndexData->mobprogs)
		if (mprg->type == MobProg::Type::BRIBE_PROG) {
			if (amount >= atoi(mprg->arglist)) {
				mprog_driver(mprg, mob, ch, obj, nullptr);
				break;
			}
		}

	extract_obj(obj);
}

void mprog_death_trigger(Character *mob)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::DEATH_PROG)))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::DEATH_PROG);

	if (!mob->is_garbage())
		death_cry(mob);

	return;
}

void mprog_entry_trigger(Character *mob)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::ENTRY_PROG)))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::ENTRY_PROG);

	return;
}

void mprog_fight_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::FIGHT_PROG)))
		mprog_percent_check(mob, ch, nullptr, nullptr, MobProg::Type::FIGHT_PROG);

	return;
}

void mprog_buy_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::BUY_PROG)))
		mprog_percent_check(mob, ch, nullptr, nullptr, MobProg::Type::BUY_PROG);

	return;
}

void mprog_give_trigger(Character *mob, Character *ch, Object *obj)
{
	String buf;

	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::GIVE_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs) {
			one_argument(mprg->arglist, buf);

			if ((mprg->type == MobProg::Type::GIVE_PROG)
			    && ((!strcmp(obj->name, mprg->arglist))
			        || (!strcmp("all", buf)))) {
				mprog_driver(mprg, mob, ch, obj, nullptr);
				break;
			}
		}

	return;
}

void mprog_greet_trigger(Character *ch)
{
	Character *vmob;

	for (vmob = ch->in_room->people; vmob != nullptr; vmob = vmob->next_in_room) {
		if (ch->is_garbage())
			break;

		if (vmob->is_npc()
		    && ch != vmob
		    && can_see_char(vmob, ch)
		    && (vmob->fighting == nullptr)
		    && IS_AWAKE(vmob)
		    && (vmob->pIndexData->progtypes.count(MobProg::Type::GREET_PROG)))
			mprog_percent_check(vmob, ch, nullptr, nullptr, MobProg::Type::GREET_PROG);
		else if (vmob->is_npc()
		         && ch != vmob
		         && (vmob->fighting == nullptr)
		         && IS_AWAKE(vmob)
		         && (vmob->pIndexData->progtypes.count(MobProg::Type::ALL_GREET_PROG)))
			mprog_percent_check(vmob, ch, nullptr, nullptr, MobProg::Type::ALL_GREET_PROG);
	}

	return;
}

void mprog_hitprcnt_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(MobProg::Type::HITPRCNT_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs)
			if ((mprg->type == MobProg::Type::HITPRCNT_PROG)
			    && ((100 * mob->hit / GET_MAX_HIT(mob)) < atoi(mprg->arglist))) {
				mprog_driver(mprg, mob, ch, nullptr, nullptr);
				break;
			}

	return;
}

void mprog_boot_trigger(Character *mob)
{
	if (mob->pIndexData->progtypes.count(MobProg::Type::BOOT_PROG))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::BOOT_PROG);

	return;
}

bool mprog_random_trigger(Character *mob)
{
	if (mob->pIndexData->progtypes.count(MobProg::Type::RAND_PROG))
		return mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::RAND_PROG);

	return false;
}

bool mprog_random_area_trigger(Character *mob)
{
	if (!mob->pIndexData->progtypes.count(MobProg::Type::RAND_AREA_PROG))
		return false;

	// this is static to avoid creating the object every time, make sure to clear it below
	static std::set<Room *> rooms;

	// build a set of all rooms in the area that have players
	for (Descriptor *d = descriptor_list; d; d = d->next) {
		if (d->is_playing()
		 && d->character->in_room
		 && d->character->in_room->area() == mob->in_room->area())
			rooms.emplace(d->character->in_room);
	}

	if (rooms.empty())
		return false;

	Room *orig_room = mob->in_room;
	char_from_room(mob);
	bool triggered = false;

	for (Room *room : rooms) {
		char_to_room(mob, room);
		if (mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::RAND_AREA_PROG))
			triggered = true;

		char_from_room(mob);

		if (mob->is_garbage()) // got killed by something?
			break;
	}

	if (!mob->is_garbage())
		char_to_room(mob, orig_room);

	rooms.clear();
	return triggered;
}

void mprog_tick_trigger(Character *mob)    /* Montrey */
{
	if (mob->pIndexData->progtypes.count(MobProg::Type::TICK_PROG))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, MobProg::Type::TICK_PROG);

	return;
}

void mprog_speech_trigger(const String& txt, Character *mob)
{
	Character *vmob;

	for (vmob = mob->in_room->people; vmob != nullptr; vmob = vmob->next_in_room) {
		if (mob->is_garbage())
			break;

		if (vmob->is_npc() && (vmob->pIndexData->progtypes.count(MobProg::Type::SPEECH_PROG)))
			mprog_wordlist_check(txt.c_str(), vmob, mob, nullptr, nullptr, MobProg::Type::SPEECH_PROG);
	}

	return;
}

void mprog_control_trigger(Character *mob, const String& key, Character *target) {
	if (mob->is_npc() && mob->pIndexData->progtypes.count(MobProg::Type::CONTROL_PROG))
		for (const auto mprg : mob->pIndexData->mobprogs) {
			if (mob->is_garbage())
				break;

			if (mprg->type == MobProg::Type::CONTROL_PROG && key == mprg->arglist)
				mprog_driver(mprg, mob, target, nullptr, nullptr);
		}
}

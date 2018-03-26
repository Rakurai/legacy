#include <map>
#include <stack>
#include "progs/Prog.hh"
#include "progs/contexts/Context.hh"
#include "progs/Operator.hh"
#include "progs/symbols/Symbol.hh"
#include "progs/prog_table.hh"
#include "file.hh"
#include "Logging.hh"


namespace progs {

Prog::
Prog(FILE *fp, Vnum vnum) {
	String name = fread_word(fp);
	type = name_to_type(name);

	arglist = fread_string(fp);
	fread_to_eol(fp);
	original = fread_string(fp);
	fread_to_eol(fp);

	if (original.empty())
		return;

	// parse the script into expressions and statements, pre-check for control
	// flow and variable reference errors

	Line::Type last_control, last_type;
	String script = original;
	int unclosed_ifs = 0;

	// make a copy of the default bindings
	std::map<String, data::Type> var_bindings(prog_table.find(type)->second.default_bindings);

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
			 && last_type != Line::Type::OR)
				throw Format::format("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));

			last_control = line_type;
			break;

		case Line::Type::AND:
			// "and" can only follow "if" or "and", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::AND)
				throw Format::format("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));

			last_control = line_type;
			break;

		case Line::Type::ELSE:
			// "else" can only follow "if" or "and" or "or" controls
			if (last_control != Line::Type::IF
			 && last_control != Line::Type::AND
			 && last_control != Line::Type::OR)
				throw Format::format("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));

			// "else" must have statements or a nested control structure in the block
			if (last_type != Line::Type::COMMAND
			 && last_type != Line::Type::BREAK
			 && last_type != Line::Type::ENDIF)
				throw Format::format("progs::Prog: 'else' has no statements to follow");

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
			 && last_type != Line::Type::ENDIF)
				throw Format::format("progs::Prog: 'endif' has no statements to follow");

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

		// test the line for variable and function usage that doesn't make sense
		// make a copy of the string
		String copy = line.lstrip();

		while (!copy.empty()) {
			if (copy[0] == '$') {
				// make sure the variable name is in the allowed list
				String copy2 = copy.substr(1); // parsing will consume this copy
				String var_name = symbols::parse_identifier(copy2);

				if (var_bindings.find(var_name) == var_bindings.cend())
					throw Format::format("progs::Prog: variable $%s is undefined for prog type '%s'", var_name, name);

				// go back to the working copy, done with copy2
				// parse variable and function stack, throws errors if it breaks
				symbols::parseVariableSymbol(copy, var_bindings);
			}
			else
				// parse until a variable start
				symbols::parseStringSymbol(copy, "$");
		}

		lines.push_back({line_type, line, var_bindings});
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

/* I didn't like the complex recursive function that was part of the original mobprog code, so
 * I rewrote it.  Run of the mill stack processor.  On encountering an 'if', we push the evaluation T/F onto
 * the stack.  On encountering commands, we look at the latest entry to decide if we're currently
 * executing those commands.  On encountering another 'if', we push its evaluation unless the parent
 * is false, in which case we push false.  On encountering 'else', we pop and push the opposite,
 * but again, if the parent is false we push false.  On encountering 'endif', we pop. -- Montrey */
void Prog::
execute(contexts::Context& context) const {
	int line_num = 0;

try {
	// make sure all bindings are correct
	// same number of bindings
	// each binding is of same type as default
	// each binding has a data wrapper defined (could be nullptr?)
	const auto& default_bindings = prog_table.find(type)->second.default_bindings;

	if (context.bindings.size() != default_bindings.size())
		throw Format::format("context has %d variables bound, default is %d",
			context.bindings.size(), default_bindings.size());

	for (const std::pair<String, data::Type>& context_pair : context.bindings) {
		const String& name = context_pair.first;
		const auto default_pair = default_bindings.find(name);

		if (default_pair == default_bindings.cend())
			throw Format::format("context variable '%s' not found in default bindings", name);

		if (context_pair.second != default_pair->second)
			throw Format::format("context variable '%s' is of type '%s', default is '%s'",
				name, data::type_to_string(context_pair.second), data::type_to_string(default_pair->second));

		if (context.vars.find(name) == context.vars.cend())
			throw Format::format("context variable '%s' is uninitialized", name);
	}

	std::stack<bool> stack;
	bool opposite;

	for (const Line& line : lines) {
		line_num++;

		switch (line.type) {
		case Line::Type::IF:
			// if we're under a false evaluation, don't even bother evaluating the line, we'll stay false
			if (!stack.empty() && stack.top() == false) {
				stack.push(false);
				continue;
			}

			// otherwise push the evaluation
			stack.push(line.expression->evaluate(context));
			break;

		case Line::Type::OR:
			if (stack.empty())
				throw String("'or' encountered without 'if'");

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
			stack.push(line.expression->evaluate(context));
			break;

		case Line::Type::AND:
			if (stack.empty())
				throw String("'and' encountered without 'if'");

			// "and" won't change a false evaluation, don't bother evaluating
			if (stack.top() == false)
				continue;

			// otherwise, pop the true parent and we'll see if it changes
			stack.pop();

			stack.push(line.expression->evaluate(context));
			break;

		case Line::Type::ELSE:
			if (stack.empty())
				throw String("'else' encountered without 'if'");

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
				throw String("'endif' encountered without 'if'");

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
	Logging::bugf("progs::execute: Exception caught in prog on %s %d, line %d:",
		context.type(), context.vnum(), line_num);
	Logging::bugf(e);
	Logging::bugf(original);
}
}

} // namespace progs


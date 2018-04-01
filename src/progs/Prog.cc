#include <map>
#include <stack>
#include "progs/Prog.hh"
#include "progs/contexts/Context.hh"
#include "progs/Operator.hh"
#include "progs/symbols/declare.hh"
#include "progs/prog_table.hh"
#include "file.hh"
#include "Logging.hh"


namespace progs {

Prog::
Prog(FILE *fp, Vnum vnum, int& line_no) {
	String name = fread_word(fp);
	type = name_to_type(name);

	arglist = fread_string(fp);
	fread_to_eol(fp);
	String script = fread_string(fp);
	fread_to_eol(fp);

	if (script.empty())
		return;

	// parse the script into expressions and statements, pre-check for control
	// flow and variable reference errors

	Line::Type last_control, last_type;
	int unclosed_ifs = 0, indent = 0;

	// make a copy of the default bindings to track illegal variable calls
	data::Bindings var_bindings(prog_table.find(type)->second.default_bindings);
	bool increase_indent_next = false;

	while (!script.empty()) {
		// can't use 'lsplit' here because we want to preserve the original lines for
		// debugging, and lsplit will strip excess newlines
		std::size_t split_pos = script.find_first_of("\n", 0);
		String orig_line;

		if (split_pos == std::string::npos) {
			orig_line.assign(script);
			script.clear();
		}
		else {
			orig_line.assign(script.substr(0, split_pos));
			script.erase(0, split_pos+1);
		}

		// grab the first token, if it's some mud command we'll put it back
		// lsplit will left strip both strings of leading whitespace
		// parse into the first word and the expression
		String word;
		String line = orig_line.strip(" \t\r").lsplit(word, " \t");

		Line::Type line_type = Line::get_type(word);
		line_no++;

		bool increase_indent_this = false;

		if (increase_indent_next) {
			increase_indent_this = true;
			increase_indent_next = false;
		}

		switch(line_type) {
		case Line::Type::EMPTY:
			line = orig_line;
			break;

		case Line::Type::IF:
			last_control = line_type;
			unclosed_ifs++;
			increase_indent_next = true;
			break;

		case Line::Type::OR:
			// "or" can only follow "if" or "or", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::OR)
				throw Format::format("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));

			increase_indent_this = false;
			increase_indent_next = true;
			last_control = line_type;
			break;

		case Line::Type::AND:
			// "and" can only follow "if" or "and", not statements or other controls
			if (last_type != Line::Type::IF
			 && last_type != Line::Type::AND)
				throw Format::format("progs::Prog: '%s' illegally following '%s'", Line::get_type(line_type), Line::get_type(last_control));

			increase_indent_this = false;
			increase_indent_next = true;
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
			 && last_type != Line::Type::ENDIF
			 && last_type != Line::Type::ASSIGN)
				throw Format::format("progs::Prog: 'else' has no statements to follow");

			// if anything else on the line (such as in 'else if ...'), put it back and parse next time
			if (!line.strip().empty()) {
				script = line + "\n" + script;
				line_no--;
				line = "";
			}

			indent--;
			increase_indent_next = true;
			last_control = line_type;
			break;

		case Line::Type::ENDIF:
			// "endif" must have statements or a nested control structure in the block
			if (last_type != Line::Type::COMMAND
			 && last_type != Line::Type::BREAK
			 && last_type != Line::Type::ENDIF
			 && last_type != Line::Type::ASSIGN)
				throw Format::format("progs::Prog: 'endif' has no statements to follow");

			// if anything else on the line (such as in 'endif endif ...'), put it back and parse next time
			if (!line.strip().empty()) {
				script = line + "\n" + script;
				line_no--;
				line = "";
			}

			indent--;
			last_control = line_type;
			unclosed_ifs--;
			break;

		case Line::Type::BREAK:
			// if anything else on the line (such as in 'endif endif ...'), put it back and parse next time
			if (!line.strip().empty()) {
				script = line + "\n" + script;
				line_no--;
				line = "";
			}

			break;

		case Line::Type::ASSIGN:
			break;			

		case Line::Type::COMMAND:
			// put the line back together with the word
			line = word + " " + line;
			break;
		}

		if (increase_indent_this)
			indent++;

		if (line_type != Line::Type::EMPTY)
			last_type = line_type;

		lines.push_back({line_type, line, indent, var_bindings});
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
try {
	// context will have default bindings for the prog type, now test that
	// the variables are initialized
	if (context.bindings.size() != (context.variables.size() + context.aliases.size()))
		throw Format::format("context has %d variables initialized, requires %d",
			context.variables.size() + context.aliases.size(), context.bindings.size());

	for (const std::pair<String, data::Type>& context_pair : context.bindings) {
		const String& name = context_pair.first;

		if (context.variables.count(name) == 0
		 && context.aliases.count(name) == 0)
			throw Format::format("context variable '%s' is bound but not initialized", name);
	}

	debug(context, Format::format("running:   %s on %s vnum %d",
		type_to_name(type), context.type(), context.vnum()));

	std::stack<bool> stack;
	bool opposite;

	for (const Line& line : lines) {

		// did the mob extract itself?
		if (context.self_is_garbage())
			return;

		context.current_line++;
		context.current_depth = line.indent;

		switch (line.type) {
		case Line::Type::EMPTY:
			continue;

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

		case Line::Type::ASSIGN:
			if (!stack.empty() && stack.top() == false)
				continue;

			line.expression->evaluate(context);
			break;

		case Line::Type::COMMAND:
			if (!stack.empty() && stack.top() == false)
				continue;

			// mud commands
			context.process_command(line.text);
			break;
		}
	}

	if (!stack.empty())
		throw String("progs::execute: reached end of script without closing 'if' statements");
} catch (String e) {
	Logging::bugf("progs::execute: Exception caught in %s on %s vnum %d, line %d:",
		type_to_name(type), context.type(), context.vnum(), context.current_line);
	Logging::bugf(e);

	if (context.current_line > 0) {
		// print the lines
		int first = std::max(context.current_line - 2, 1);
		int last  = std::min(context.current_line + 2, (int)lines.size());
		for (int i = first; i <= last; i++) {
			String buf = lines[i-1].pretty_print().replace("$", "$$");
			Logging::bugf("%s%s{x", i == context.current_line ? "{Y" : "", buf);
		}
	}
}
}

} // namespace progs


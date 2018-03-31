#include "progs/symbols/declare.hh"
#include "progs/symbols/Symbol.hh"
#include "progs/contexts/Context.hh"
#include "Character.hh"
#include "Object.hh"
#include "Room.hh"

namespace progs {
namespace symbols {

const String var_to_string(World * var) { return var ? "World" : "0"; }
const String var_to_string(Character * var) { return var ? var->name : "0"; }
const String var_to_string(Object * var) { return var ? var->name : "0"; }
const String var_to_string(Room * var) { return var ? var->name() : "0"; }
const String var_to_string(bool var) { return var ? "1" : "0"; }
const String var_to_string(int var) { return Format::format("%d", var); }
const String var_to_string(const String& var) { return var; }

const String
expand(const String& orig, contexts::Context& context) {
	String copy = orig, buf;

	while (!copy.empty()) {
		if (copy[0] == '$') {
			std::unique_ptr<Symbol> ptr = parse(copy, context.bindings, "");

			if (ptr->type == data::Type::Character
			 || ptr->type == data::Type::Object
			 || ptr->type == data::Type::Room) {
				String fn = "name()";
				ptr = parseFunctionSymbol(fn, context.bindings, ptr);
			}

			buf += ptr->to_string(context);
		}
		else {
			buf += copy[0];
			copy.erase(0, 1);
		}
	}

	return buf;
}

} // namespace symbols
} // namespace progs

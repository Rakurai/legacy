#include "progs/contexts/Context.hh"
#include "progs/symbols/Symbol.hh"
#include "progs/symbols/declare.hh"

namespace progs {
namespace contexts {

Context::
~Context() {
	for (auto& pair : vars)
		delete pair.second;
}

Context::
Context(const Context& rhs) {
	for (auto& pair : rhs.vars)
		vars.emplace(pair.first, pair.second->clone());
}

const String Context::
expand_vars(const String& orig) {
	String copy = orig, buf;

	while (!copy.empty()) {
		if (copy[0] == '$') {
			std::unique_ptr<symbols::Symbol> ptr = symbols::parse(copy, bindings, "");

			if (ptr->type == data::Type::Character
			 || ptr->type == data::Type::Object
			 || ptr->type == data::Type::Room) {
				String fn = "name()";
				ptr = symbols::parseFunctionSymbol(fn, bindings, ptr);
			}

			buf += ptr->to_string(*this);
		}
		else {
			buf += copy[0];
			copy.erase(0, 1);
		}
	}

	return buf;
}

} // namespace contexts
} // namespace progs

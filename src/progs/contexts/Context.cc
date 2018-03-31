#include "progs/contexts/Context.hh"

namespace progs {
namespace contexts {

Context::
~Context() {
	for (auto& pair : variables)
		delete pair.second;
}

Context::
Context(const Context& rhs) {
	for (auto& pair : rhs.aliases)
		aliases.emplace(pair.first, pair.second);

	for (auto& pair : rhs.bindings)
		bindings.emplace(pair.first, pair.second);

	for (auto& pair : rhs.variables)
		variables.emplace(pair.first, pair.second->clone());
}

} // namespace contexts
} // namespace progs

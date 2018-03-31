#include "progs/contexts/Context.hh"

namespace progs {
namespace contexts {

Context::
~Context() {
	for (auto& pair : vars)
		delete pair.second;
}

Context::
Context(const Context& rhs) {
	for (auto& pair : rhs.bindings)
		bindings.emplace(pair.first, pair.second);

	for (auto& pair : rhs.vars)
		vars.emplace(pair.first, pair.second->clone());
}

} // namespace contexts
} // namespace progs

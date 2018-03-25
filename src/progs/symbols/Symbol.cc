#include "progs/symbols/deref.hh"

namespace progs {
namespace symbols {

void Symbol::
execute(contexts::Context& context) {
	switch (type) {
		case Symbol::Type::Character: deref<Character *>(this, context); break;
		case Symbol::Type::Object:    deref<Object *>(this, context); break;
		case Symbol::Type::String:    deref<const String>(this, context); break;
		case Symbol::Type::Boolean:   deref<bool>(this, context); break;
		case Symbol::Type::Integer:   deref<int>(this, context); break;
		case Symbol::Type::Void:      deref<int>(this, context); break;
		case Symbol::Type::global:
		case Symbol::Type::unknown:
			throw Format::format("progs::Symbol::execute: unable to execute symbol stack");
	}
}

} // namespace symbols
} // namespace progs

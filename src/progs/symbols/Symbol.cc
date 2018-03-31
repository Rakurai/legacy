#include "progs/symbols/templates.hh"

namespace progs {
namespace symbols {

void Symbol::
execute(contexts::Context& context) {
	switch (type) {
		case data::Type::World:     break; // nothing returns world, don't bother executing
		case data::Type::Character: deref<Character *>(this, context); break;
		case data::Type::Object:    deref<Object *>(this, context); break;
		case data::Type::Room:      deref<Room *>(this, context); break;
		case data::Type::String:    deref<String>(this, context); break;
		case data::Type::Boolean:   deref<bool>(this, context); break;
		case data::Type::Integer:   deref<int>(this, context); break;
		case data::Type::Void:      deref<int>(this, context); break;
	}
}

} // namespace symbols
} // namespace progs

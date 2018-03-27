#pragma once

#include "String.hh"

namespace progs {
namespace data {

// Object types that variable and function symbols can return or take as arguments
// helps avoid a long series of dynamic cast attempts to determine an object type
enum class Type {
	World,
	Character,
	Object,
	Room,
	String,
	Boolean,
	Integer,

	Void,
};

inline const String type_to_string(Type type) {
	switch (type) {
		case Type::World:     return "World";
		case Type::Character: return "Character";
		case Type::Object:    return "Object";
		case Type::Room:      return "Room";
		case Type::String:    return "String";
		case Type::Boolean:   return "Boolean";
		case Type::Integer:   return "Integer";
		case Type::Void:      return "Void";
	}

	return "unknown";
}

} // namespace data
} // namespace progs

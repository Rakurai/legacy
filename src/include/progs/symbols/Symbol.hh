#pragma once

#include "progs/contexts/declare.hh"
#include "String.hh"

namespace progs {
namespace symbols {

struct Symbol {
	// Object types that variable and function symbols can return or take as arguments
	// helps avoid a long series of dynamic cast attempts to determine an object type
	enum class Type {
		Character,
		Object,
		String,
		Boolean,
		Integer,
		Void,

		global,
		unknown
	};

	const String type_to_string() const {
		switch (type) {
			case Symbol::Type::Character: return "Character";
			case Symbol::Type::Object: return "Object";
			case Symbol::Type::String: return "String";
			case Symbol::Type::Boolean: return "Boolean";
			case Symbol::Type::Integer: return "Integer";
			case Symbol::Type::Void:    return "Void";
			case Symbol::Type::global: return "global";
			case Symbol::Type::unknown: return "unknown";
		}
	}

	Symbol(Type s) : type(s) {}
	virtual ~Symbol() { }

	virtual const String to_string(contexts::Context&) = 0;
	virtual const String print_stack() const = 0;

	void execute(contexts::Context&);

	Type type;
};

} // namespace symbols
} // namespace progs

#pragma once

#include <memory>
#include "progs/Expression.hh"
#include "String.hh"

namespace progs {

struct Line {
	enum class Type {
		IF,
		AND,
		OR,
		ELSE,
		ENDIF,
		BREAK,
		ASSIGN,
		COMMAND,
		EMPTY,
	};

	Line(Type type, const String& text, const String& orig_text, data::Bindings& var_bindings);

	Type type;
	String text;
	String orig_text;
	std::unique_ptr<Expression> expression;

	static Type get_type(const String& word) {
		if (word.empty() || word[0] == '*') return Type::EMPTY;
		else if (word == "if")     return Type::IF;
		else if (word == "and")    return Type::AND;
		else if (word == "or")     return Type::OR;
		else if (word == "else")   return Type::ELSE;
		else if (word == "endif")  return Type::ENDIF;
		else if (word == "break")  return Type::BREAK;
		else if (word == "assign") return Type::ASSIGN;

		return Type::COMMAND;
	}

	static const String get_type(Type type) {
		switch (type) {
			case Type::IF:      return "if";
			case Type::AND:     return "and";
			case Type::OR:      return "or";
			case Type::ELSE:    return "else";
			case Type::ENDIF:   return "endif";
			case Type::BREAK:   return "break";
			case Type::COMMAND: return "command";
			case Type::ASSIGN:  return "assign";
			case Type::EMPTY:   return "empty";
		}

		return "unknown";
	}
};

} // namespace progs

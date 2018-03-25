#pragma once

#include <vector>
#include "progs/declare.hh"
#include "progs/contexts/declare.hh"
#include "String.hh"

class Character;
class Object;

namespace progs {

class Operator;

namespace symbols {

struct Symbol;
template <typename T> struct ValueSymbol;
template <typename T> struct VariableSymbol;
template <typename T> struct FunctionSymbol;

const String var_to_string(Character * var);
const String var_to_string(Object * var);
const String var_to_string(bool var);
const String var_to_string(int var);
const String var_to_string(const String& var);

bool evaluate(const Operator&, const std::unique_ptr<Symbol>& lhs,
	const std::unique_ptr<Symbol>&rhs, contexts::Context&);

// factory method
std::unique_ptr<Symbol> parse(String&, const String& until);
std::unique_ptr<Symbol> parseVariableSymbol(String&);
std::unique_ptr<Symbol> parseFunctionSymbol(String&, std::unique_ptr<Symbol>&);
std::unique_ptr<Symbol> parseIntegerSymbol(String&);
std::unique_ptr<Symbol> parseBooleanSymbol(String&);
std::unique_ptr<Symbol> parseStringSymbol(String&, const String& until);

} // namespace symbols
} // namespace progs

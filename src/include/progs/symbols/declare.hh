#pragma once

#include <memory>
#include <vector>
#include <map>
#include "progs/declare.hh"
#include "progs/contexts/declare.hh"
#include "progs/data/Type.hh"
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
std::unique_ptr<Symbol> parse(String&, const std::map<String, data::Type>& var_bindings, const String& until);
const String parse_identifier(String&);
std::unique_ptr<Symbol> parseVariableSymbol(String&, const std::map<String, data::Type>& var_bindings);
std::unique_ptr<Symbol> parseFunctionSymbol(String&, const std::map<String, data::Type>& var_bindings, std::unique_ptr<Symbol>&);
std::unique_ptr<Symbol> parseIntegerSymbol(String&);
std::unique_ptr<Symbol> parseBooleanSymbol(String&);
std::unique_ptr<Symbol> parseStringSymbol(String&, const String& until);

} // namespace symbols
} // namespace progs

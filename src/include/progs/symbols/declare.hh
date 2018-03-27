#pragma once

#include <memory>
#include <vector>
#include <map>
#include "progs/declare.hh"
#include "progs/contexts/declare.hh"
#include "progs/data/Type.hh"
#include "progs/data/Bindings.hh"
#include "String.hh"

class Character;
class Object;

namespace progs {
namespace symbols {

struct Symbol;
template <typename T> struct ValueSymbol;
template <typename T> struct VariableSymbol;
template <typename T> struct FunctionSymbol;

const String var_to_string(Character *);
const String var_to_string(Object *);
const String var_to_string(Room *);
const String var_to_string(bool);
const String var_to_string(int);
const String var_to_string(const String&);

// factory method
std::unique_ptr<Symbol> parse(String&, const data::Bindings&, const String& until);
std::unique_ptr<Symbol> parseVariableSymbol(String&, const data::Bindings&);
std::unique_ptr<Symbol> parseFunctionSymbol(String&, const data::Bindings&, std::unique_ptr<Symbol>&);
std::unique_ptr<Symbol> parseIntegerSymbol(String&);
std::unique_ptr<Symbol> parseBooleanSymbol(String&);
std::unique_ptr<Symbol> parseStringSymbol(String&, const String& until);
const String parse_identifier(String&);

} // namespace symbols
} // namespace progs

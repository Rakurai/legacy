#include "Character.hh"
#include "Object.hh"
#include "Room.hh"

namespace progs {
namespace symbols {

const String var_to_string(World * var) { return var ? "World" : "0"; }
const String var_to_string(Character * var) { return var ? var->name : "0"; }
const String var_to_string(Object * var) { return var ? var->name : "0"; }
const String var_to_string(Room * var) { return var ? var->name() : "0"; }
const String var_to_string(bool var) { return var ? "1" : "0"; }
const String var_to_string(int var) { return Format::format("%d", var); }
const String var_to_string(const String& var) { return var; }

} // namespace symbols
} // namespace progs

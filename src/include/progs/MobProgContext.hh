#pragma once

#include "progs/Context.hh"
#include "String.hh"

class Character;
class Object;

namespace progs {

class MobProgContext :
public Context 
{
public:
	MobProgContext(Character *mob, Character *actor, Object *obj, void *vo);
	virtual ~MobProgContext() {}

	Character *get_char_target(const String&) const;
	Object *get_obj_target(const String&) const;
	String translate(char ch) const;

	virtual const Vnum vnum() const;

	virtual const String dereference_variable(const String& var, String member_name) const;
	virtual const String compute_function(const String& fn, const std::vector<String>& args) const;
	virtual void process_command(const String& cmnd) const;
	virtual bool self_is_garbage() const;

	Character *mob;
// implicit copy and assignment constructors allowed
};

} // namespace progs

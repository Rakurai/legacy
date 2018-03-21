#pragma once

#include <vector>
#include "String.hh"
#include "Vnum.hh"
#include "progs/Symbol.hh"

class Character;
class Object;

namespace progs {

class Context {
public:
	virtual const Vnum vnum() const = 0;

	// local
	const String access_member(const Character *, const String& member_name, bool can_see) const;
	const String access_member(const Object *, const String& member_name, bool can_see) const;

	// can be overridden
	virtual const String compute_function(const String& fn, const std::vector<std::unique_ptr<Symbol>>& arg_list) const;

	// must be overridden
	virtual Character *get_char_target(const std::unique_ptr<Symbol>) const = 0;
	virtual Object *get_obj_target(const std::unique_ptr<Symbol>) const = 0;
	virtual const String dereference_variable(String var, String member_name) const = 0;
	virtual void process_command(const String& cmnd) const = 0;
	virtual bool self_is_garbage() const = 0;

	void *self;
	Character *actor;
	Object *obj;
	void *vo;
	Character *rndm;
};

} // namespace progs

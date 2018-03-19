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

	virtual const String dereference_variable(const String& var, String member_name) const = 0;
	virtual const String compute_function(const String& fn, const std::vector<String>& args) const = 0;
	virtual void process_command(const String& cmnd) const = 0;
	virtual bool self_is_garbage() const = 0;

	void *self;
	Character *actor;
	Object *obj;
	void *vo;
	Character *rndm;
};

} // namespace progs

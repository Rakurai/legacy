#pragma once

#include "progs/contexts/Context.hh"

namespace progs {
namespace contexts {

class ObjProgContext :
public Context 
{
public:
	ObjProgContext(progs::Type, Object *obj);
	virtual ~ObjProgContext() {}

	// overrides
	virtual const String type() const { return "obj"; }
	virtual const Vnum vnum() const;
	virtual bool can_see(Character *) const;
	virtual bool can_see(Object *) const;
	virtual bool can_see(Room *) const;
	virtual void process_command(const String& cmnd);
	virtual bool self_is_garbage() const;

	Object *obj;

private:
	ObjProgContext& operator=(const ObjProgContext&);
};

} // namespace contexts
} // namespace progs

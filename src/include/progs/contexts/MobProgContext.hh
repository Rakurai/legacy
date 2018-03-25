#pragma once

#include "progs/contexts/Context.hh"

namespace progs {
namespace contexts {

class MobProgContext :
public Context 
{
public:
	MobProgContext(Character *mob);
	virtual ~MobProgContext() {}

	virtual const String type() const { return "mob"; }
	virtual const Vnum& vnum() const;;
	virtual bool can_see(Character *ch) const;
	virtual bool can_see(Object *obj) const;

	// overrides
	virtual void process_command(const String& cmnd);
	virtual bool self_is_garbage() const;

	Character *mob;

private:
	MobProgContext& operator=(const MobProgContext&);
};

} // namespace contexts
} // namespace progs

#pragma once

#include "progs/contexts/Context.hh"

namespace progs {
namespace contexts {

class MobProgContext :
public Context 
{
public:
	MobProgContext(progs::Type, Character *mob);
	virtual ~MobProgContext() {}

	// overrides
	virtual const String type() const { return "mob"; }
	virtual const Vnum vnum() const;
	virtual bool can_see(Character *) const;
	virtual bool can_see(Object *) const;
	virtual bool can_see(Room *) const;
	virtual Room *in_room() const;

	virtual void process_command(const String& cmnd);
	virtual bool self_is_garbage() const;

	Character *mob;

private:
	MobProgContext& operator=(const MobProgContext&);
};

} // namespace contexts
} // namespace progs

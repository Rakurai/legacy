#pragma once

#include "String.hh"
#include "MobProgContext.hh"

namespace progs {

class MobProgActList
{
public:
	MobProgActList(const String& s, const MobProgContext& c) : buf(s), context(c) {}
	virtual ~MobProgActList() {}

    MobProgActList * next = nullptr;
    String           buf;
    MobProgContext   context;

private:
	MobProgActList(const MobProgActList&);
	MobProgActList& operator=(const MobProgActList&);
};

} // namespace progs

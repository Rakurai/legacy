#pragma once

#include "progs/contexts/MobProgContext.hh"
#include "String.hh"

namespace progs {

class MobProgActList
{
public:
	MobProgActList(const String& s, const contexts::MobProgContext& c) : buf(s), context(c) {}
	virtual ~MobProgActList() {}

    MobProgActList * next = nullptr;
    String           buf;
    contexts::MobProgContext   context;

private:
	MobProgActList(const MobProgActList&);
	MobProgActList& operator=(const MobProgActList&);
};

} // namespace progs

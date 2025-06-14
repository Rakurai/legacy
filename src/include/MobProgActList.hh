#pragma once

#include "String.hh"

class Character;
class Object;

class MobProgActList
{
public:
	MobProgActList() {}
	virtual ~MobProgActList() {}

    MobProgActList * next = nullptr;
    String           buf;
    Character *      ch = nullptr;
    Object *         obj = nullptr;
    void *           vo = nullptr;

private:
	MobProgActList(const MobProgActList&);
	MobProgActList& operator=(const MobProgActList&);
};

#pragma once

class MobProgActList
{
public:
	MobProgActList() {}
	virtual ~MobProgActList() {}

    MobProgActList * next = NULL;
    String           buf;
    Character *      ch = NULL;
    Object *         obj = NULL;
    void *           vo = NULL;

private:
	MobProgActList(const MobProgActList&);
	MobProgActList& operator=(const MobProgActList&);
};

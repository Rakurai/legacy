#pragma once

/* Data for generating characters -- only used during generation */
class Customize
{
public:
	Customize() {}
	virtual ~Customize() {}

    bool        valid = FALSE;
    bool        skill_chosen[MAX_SKILL] = {FALSE};
    bool        group_chosen[MAX_GROUP] = {FALSE};
    int         points_chosen = 0;

private:
	Customize(const Customize&);
	Customize& operator=(const Customize&);
};

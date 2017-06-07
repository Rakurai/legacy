#pragma once

#include "declare.h"
#include <vector>

/* Data for generating characters -- only used during generation */
class Customize
{
public:
	Customize();
	virtual ~Customize() {}

    bool        valid = FALSE;
    std::vector<bool> skill_chosen;
    std::vector<bool> group_chosen;
    int         points_chosen = 0;

private:
	Customize(const Customize&);
	Customize& operator=(const Customize&);
};

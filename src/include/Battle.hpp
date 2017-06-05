#pragma once

#include "declare.h"

class Battle
{
public:
	Battle() {}
	virtual ~Battle() {}

    int         low = 0;
    int         high = 0;
    int         fee = 0;
    bool        issued = FALSE;
    bool        start = FALSE;

private:
	Battle(const Battle&);
	Battle& operator=(const Battle&);
};

extern Battle battle;

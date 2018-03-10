#pragma once

class Battle
{
public:
	Battle() {}
	virtual ~Battle() {}

    int         low = 0;
    int         high = 0;
    int         fee = 0;
    bool        issued = false;
    bool        start = false;

private:
	Battle(const Battle&);
	Battle& operator=(const Battle&);
};

extern Battle battle;

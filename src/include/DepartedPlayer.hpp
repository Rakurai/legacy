#pragma once

class DepartedPlayer
{   
public:
	DepartedPlayer() {}
	virtual ~DepartedPlayer() {}

    String name;
    DepartedPlayer *next = NULL;
    DepartedPlayer *previous = NULL;

private:
	DepartedPlayer(const DepartedPlayer&);
	DepartedPlayer& operator=(const DepartedPlayer&);
};

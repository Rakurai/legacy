#pragma once

class DepartedPlayer
{   
public:
	DepartedPlayer() {}
	virtual ~DepartedPlayer() {}

    String name;
    DepartedPlayer *next = nullptr;
    DepartedPlayer *previous = nullptr;

private:
	DepartedPlayer(const DepartedPlayer&);
	DepartedPlayer& operator=(const DepartedPlayer&);
};

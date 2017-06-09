#pragma once

class StoredPlayer
{   
public:
	StoredPlayer() {}
	virtual ~StoredPlayer() {}

    String name;
    String by_who;
    String date;

    StoredPlayer *next = nullptr;
    StoredPlayer *previous = nullptr;

private:
	StoredPlayer(const StoredPlayer&);
	StoredPlayer& operator=(const StoredPlayer&);
};

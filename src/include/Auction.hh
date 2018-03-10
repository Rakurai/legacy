#pragma once

class Object;
class Character;

class Auction
{
public:
	Auction() {}
	virtual ~Auction() {}

    Object  *   item = nullptr;   /* a pointer to the item */
    Character * seller = nullptr; /* a pointer to the seller - which may NOT quit */
    Character * buyer = nullptr;  /* a pointer to the buyer - which may NOT quit */
    int         bet = 0;    /* last bet - or 0 if noone has bet anything */
    int      going = 0;  /* 1,2, sold */
    int      pulse = 0;  /* how many pulses (.25 sec) until another call-out ? */
    int         min = 0;    /* Minimum bid */

	void init();
	void update();
	bool is_participant(Object *obj) const;
	bool is_participant(Character *ch) const;

private:
	Auction(const Auction&);
	Auction& operator=(const Auction&);
};

// global auction
extern Auction auction;

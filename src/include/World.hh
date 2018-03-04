#pragma once

#include <vector>
#include <map>

#include "GameTime.hh"
#include "Weather.hh"
#include "QuestArea.hh"
#include "worldmap/Quadtree.hh"
#include "worldmap/Worldmap.hh"
#include "GarbageCollectingList.hh"
#include "quest/Quest.hh"

class Area;
class Character;
class Player;
class Object;
class Room;
class ObjectPrototype;
class MobilePrototype;
class RoomPrototype;
class Game;
class Vnum;
class Location;

// helps lookups in the areas map
struct VnumRange {
	VnumRange(Vnum mn, Vnum mx) : min(mn), max(mx) {}
	inline friend bool operator<(const VnumRange& lhs, const VnumRange& rhs) { return lhs.max < rhs.min; }
	Vnum min, max;
};

class World
{
public:
	virtual ~World();

	void update();

	GameTime time;
	Weather weather;
	QuestArea quest;
	GarbageCollectingList<Character *> char_list; // owned here
	Object *object_list = nullptr;

	Object *donation_pit = nullptr;;

	std::map<VnumRange, Area *> areas;
	std::map<String, quest::Quest> quests;

	Area *get_area(const Vnum&) const;
	bool valid() const {
		return (
			overworld.height() > 0
		 && overworld.width() > 0
		);
	}

	worldmap::Worldmap overworld;
	worldmap::Quadtree<Room> maptree;

	void get_minimap(Character *ch, std::vector<String>& v) const;

    ObjectPrototype *get_obj_prototype(const Vnum&);
    MobilePrototype *get_mob_prototype(const Vnum&);
    RoomPrototype *get_room_prototype(const Vnum&);
	Room *get_room(const Location&);

	void add_char(Character *);
	void remove_char(Character *);

	void load_areas();
	void create_rooms();
	void create_exits();
	void load_quests();

private:
	World();
	World(const World&);
	World& operator=(const World&);

	friend class Game;
};

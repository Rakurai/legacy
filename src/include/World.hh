#pragma once

#include <vector>

#include "GameTime.hh"
#include "Weather.hh"
#include "QuestArea.hh"
#include "worldmap/Quadtree.hh"
#include "worldmap/Worldmap.hh"

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

class World
{
public:
	virtual ~World();

	void update();

	GameTime time;
	Weather weather;
	QuestArea quest;
	Character *char_list = nullptr;
	Player *pc_list = nullptr;
	Object *object_list = nullptr;

	Object *donation_pit = nullptr;;

	std::vector<Area *> areas;

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

	void load_areas();
	void create_rooms();
	void create_exits();

private:
	World();
	World(const World&);
	World& operator=(const World&);

	friend class Game;
};

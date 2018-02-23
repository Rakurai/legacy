#pragma once

#include <vector>
#include <map>
#include "declare.hh"
#include "String.hh"
#include "Vnum.hh"
#include "Location.hh"

class Area
{
public:
    Area(World& w, FILE *fp);
    virtual ~Area();

    void load_rooms(FILE *fp);
    void load_mobiles(FILE *fp);
    void load_objects(FILE *fp);
    void load_region(FILE *fp);// { region = new worldmap::Region(*this, fp); }
    void create_rooms();

    ObjectPrototype *get_obj_prototype(const Vnum&);
    MobilePrototype *get_mob_prototype(const Vnum&);
    RoomPrototype *get_room_prototype(const Vnum&);

    Room *get_room(const RoomID& id) {
        auto entry = rooms.find(id);
        return entry == rooms.end() ? nullptr : entry->second;
    }

    void update();
    void reset();

    int num_players() const { return _num_players; }
    int num_imms() const { return _num_imms; }

    // components
    World& world;

    String              file_name;
    String              name;
    String              credits;
    const Vnum          min_vnum;
    const Vnum          max_vnum;

    std::vector<Reset *> resets;
    std::map<Vnum, RoomPrototype *> room_prototypes;
    std::map<Vnum, MobilePrototype *> mob_prototypes;
    std::map<Vnum, ObjectPrototype *> obj_prototypes;

    std::map<RoomID, Room *> rooms;


    // descriptive vars
    String              author;     /* -- Elrac */
    String              title;      /* -- Elrac */
    String              keywords;   /* -- Elrac */
    int              version = 1;    /* Area file versioning -- Montrey */
    char                area_type = 0;  /* -- Elrac */
    int              low_range = 0;
    int              high_range = 0;

    // state
    int              age = 15;

    worldmap::Region *region = nullptr;

    void add_char(Character *ch);
    void remove_char(Character *ch);

    friend bool operator==(const Area&, const Area&);
    friend bool operator!=(const Area&, const Area&);

private:
    Area();
    Area(const Area&);
    Area& operator=(const Area&);

    int scan_credits();

    /* pick a random room to reset into -- Montrey */
    Room *get_random_reset_room(const MobilePrototype *);

    // keep track of both imms and players.  imms shouldn't prevent an
    // area from resetting, but we also want the area to animate while
    // only an imm is there.
    int _num_players = 0;
    int _num_imms = 0;
};


inline bool operator==(const Area& lhs, const Area& rhs) {
    // all kinds of ways this could be done, but what uniquely identifies
    // an area better than its vnum range?
    return lhs.min_vnum == rhs.min_vnum && lhs.max_vnum == rhs.max_vnum;
}

inline bool operator!=(const Area& lhs, const Area& rhs) {
    return !(lhs == rhs);
}

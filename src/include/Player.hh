#pragma once

#include <vector>
#include <map>
#include <set>
#include "constants.hh"
#include "String.hh"
#include "Flags.hh"
#include "skill/skill.hh"
#include "Vnum.hh"
#include "Location.hh"
#include "quest/State.hh"

/*
 * Data which only PC's have.
 */
class Player
{
public:
	Player(Character &);
	virtual ~Player() {}

	Character &	        ch;	/* i may be missing something, but this seems like a 'duh',
				   make it point backwards to the character -- Montrey */

	long				id = 0;
	String              buffer;
	String              pwd;
	String              bamfin;
	String              bamfout;
	String              gamein;
	String              gameout;
	String              afk;
	String              title;
	String              immname;   /* Immortal Name */
    String              immprefix; // immtalk prefix
	time_t			    last_note = 0;
	time_t			    last_idea = 0;
	time_t			    last_roleplay = 0;
	time_t			    last_immquest = 0;
	time_t			    last_changes = 0;
	time_t			    last_personal = 0;
	time_t			    last_trade = 0;
	int			    trains_to_hit = 0;
	int			    trains_to_mana = 0;
	int			    trains_to_stam = 0;
	int			    mud_exp = 0;
	int                 pckills = 0;
	int                 pckilled = 0;
	int              arenakills = 0;
	int              arenakilled = 0;
	int			        pkrank = 0;
	int                 last_level = 0;
	int			        last_logoff = 0;
	int                 played = 0;
	int                 backup = 0;
	int              condition      [4] = {0};
	std::vector<int> learned;
	std::vector<int> evolution;

    Vnum                quest_giver = 0;  /* Elrac */
    int                 questpoints = 0;  /* Vassago */
    int			        questpoints_donated = 0; /* Clerve */
    long		        gold_donated = 0; /* Montrey */
    int              nextquest = 0;    /* Vassago */
    int              countdown = 0;    /* Vassago */
    Vnum             questobj = 0;     /* Vassago */
    Vnum             questmob = 0;     /* Vassago */
    Location             questloc;     /* -- Elrac */
    int              questobf = 0;     /* Lotus */

	int			        skillpoints = 0;
	Vnum                 squest_giver = 0;
	int              nextsquest = 0;
	int              sqcountdown = 0;
	Object  *           squestobj = nullptr;
	Character *         squestmob = nullptr;
	bool                squestobjf = false;
	bool		        squestmobf = false;
	Location	        squestloc1; /* obj */
	Location	        squestloc2;	/* mob */

	std::vector<bool>   group_known;
	int			        rolepoints = 0;
	int              points = 0;
	int              confirm_delete = 0;
	std::map<String, String> alias;
	std::vector<String> query;
	std::vector<String> ignore;
	Character *         skeleton = nullptr;
	Character *         zombie = nullptr;
	Character *         wraith = nullptr;
	Character *         gargoyle = nullptr;
	int              color          [MAX_COLORS] = {0};
	int              bold           [MAX_COLORS] = {0};
	Flags               cgroup_flags;
    Flags               plr_flags; /* Extra PLR flags */
    String              rank;
    int              lastcolor      [2] = {0};
    int              pktimer = 0;
    int              combattimer = 0;
    String              status;
    String              deity;
    int              remort_count = 0;
    skill::type	        extraclass     [MAX_EXTRACLASS_SLOTS] = {skill::type::unknown};
    int			        raffect        [MAX_RAFFECT_SLOTS] = {0};
    Object *            locker = nullptr;
    Object *            strongbox = nullptr;
    String              email;
    time_t              last_ltime = 0;
    time_t              last_saved = 0;
    int			    flag_thief = 0;
    int			    flag_killer = 0;
    String              last_lsite;
    String              fingerinfo;
    String              spouse;
    String              propose;
    String              whisper;
    Flags               video_flags;
    int              tailing = 0;
    Location                 mark_room;
    String              aura;
    Duel *	        duel = nullptr;
    int              lays = 0;
    int              next_lay_countdown = 0;
    bool                familiar = false;
    std::set<String>    warp_locs;

    std::vector<quest::State> quests;

private:
	Player(const Player&);
	Player& operator=(const Player&);
};

long get_pc_id();

/* mudding experience for newbies */
#define MEXP_TOTAL_NEWBIE	0
#define MEXP_LEGACY_NEWBIE	1
#define MEXP_LEGACY_OLDBIE	2

#pragma once

#include <vector>
#include <map>
#include <set>
#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "Pooled.hh"

/*
 * Data which only PC's have.
 */
class Player :
public Pooled<Player>
{
public:
	Player();
	virtual ~Player() {}

	Player *            next = nullptr;
	Character *	        ch = nullptr;	/* i may be missing something, but this seems like a 'duh',
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
	sh_int			    trains_to_hit = 0;
	sh_int			    trains_to_mana = 0;
	sh_int			    trains_to_stam = 0;
	sh_int			    mud_exp = 0;
	int                 pckills = 0;
	int                 pckilled = 0;
	sh_int              arenakills = 0;
	sh_int              arenakilled = 0;
	int			        pkrank = 0;
	int                 last_level = 0;
	int			        last_logoff = 0;
	int                 played = 0;
	int                 backup = 0;
	sh_int              condition      [4] = {0};
	std::vector<sh_int> learned;
	std::vector<sh_int> evolution;
	int			        skillpoints = 0;
	int                 squest_giver = 0;
	sh_int              nextsquest = 0;
	sh_int              sqcountdown = 0;
	Object  *           squestobj = nullptr;
	Character *         squestmob = nullptr;
	bool                squestobjf = FALSE;
	bool		        squestmobf = FALSE;
	int			        squestloc1 = 0; /* obj */
	int			        squestloc2 = 0;	/* mob */
	std::vector<bool>   group_known;
	int			        rolepoints = 0;
	sh_int              points = 0;
	sh_int              confirm_delete = 0;
	std::map<String, String> alias;
	std::vector<String> query;
	std::vector<String> ignore;
	Character *         skeleton = nullptr;
	Character *         zombie = nullptr;
	Character *         wraith = nullptr;
	Character *         gargoyle = nullptr;
	sh_int              color          [MAX_COLORS] = {0};
	sh_int              bold           [MAX_COLORS] = {0};
	Flags               cgroup_flags;
    Flags               plr_flags; /* Extra PLR flags */
    String              rank;
    sh_int              lastcolor      [2] = {0};
    sh_int              pktimer = 0;
    sh_int              combattimer = 0;
    String              status;
    String              deity;
    sh_int              remort_count = 0;
    int			        extraclass     [MAX_EXTRACLASS_SLOTS] = {0};
    int			        raffect        [MAX_RAFFECT_SLOTS] = {0};
    Object *            locker = nullptr;
    Object *            strongbox = nullptr;
    String              email;
    time_t              last_ltime = 0;
    time_t              last_saved = 0;
    sh_int			    flag_thief = 0;
    sh_int			    flag_killer = 0;
    String              last_lsite;
    String              fingerinfo;
    String              spouse;
    String              propose;
    String              whisper;
    Flags               video_flags;
    sh_int              tailing = 0;
    int                 mark_room = 0;
    String              aura;
    Duel *	        duel = nullptr;
    sh_int              lays = 0;
    sh_int              next_lay_countdown = 0;
    bool                familiar = FALSE;
    std::set<String>    warp_locs;

private:
	Player(const Player&);
	Player& operator=(const Player&);
};

long get_pc_id();

/* mudding experience for newbies */
#define MEXP_TOTAL_NEWBIE	0
#define MEXP_LEGACY_NEWBIE	1
#define MEXP_LEGACY_OLDBIE	2

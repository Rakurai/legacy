#pragma once

class Duel;

/*
 * Data which only PC's have.
 */
class Player
{
public:
	Player() {}
	virtual ~Player() {}

	Player *            next = NULL;
	Character *	        ch = NULL;	/* i may be missing something, but this seems like a 'duh',
				   make it point backwards to the character -- Montrey */

	String              buffer;
	bool                valid = FALSE;
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
	sh_int              learned        [MAX_SKILL] = {0};
	sh_int		        evolution      [MAX_SKILL] = {0};
	int			        skillpoints = 0;
	int                 squest_giver = 0;
	sh_int              nextsquest = 0;
	sh_int              sqcountdown = 0;
	Object  *           squestobj = NULL;
	Character *         squestmob = NULL;
	bool                squestobjf = FALSE;
	bool		        squestmobf = FALSE;
	int			        squestloc1 = 0; /* obj */
	int			        squestloc2 = 0;	/* mob */
	bool                group_known    [MAX_GROUP] = {0};
	int			        rolepoints = 0;
	sh_int              points = 0;
	sh_int              confirm_delete = 0;
	std::map<String, String> alias;
	std::vector<String> query;
	std::vector<String> ignore;
	Character *         skeleton = NULL;
	Character *         zombie = NULL;
	Character *         wraith = NULL;
	Character *         gargoyle = NULL;
	sh_int              color          [MAX_COLORS] = {0};
	sh_int              bold           [MAX_COLORS] = {0};
	unsigned long       cgroup = 0;
    unsigned long       plr = 0; /* Extra PLR flags */
    String              rank;
    sh_int              lastcolor      [2] = {0};
    sh_int              pktimer = 0;
    sh_int              combattimer = 0;
    String              status;
    String              deity;
    sh_int              remort_count = 0;
    int			        extraclass     [MAX_EXTRACLASS_SLOTS] = {0};
    int			        raffect        [MAX_RAFFECT_SLOTS] = {0};
    Object *            locker = NULL;
    Object *            strongbox = NULL;
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
    long                video = 0;
    sh_int              tailing = 0;
    int                 mark_room = 0;
    String              aura;
    Duel *	        duel = NULL;
    sh_int              lays = 0;
    sh_int              next_lay_countdown = 0;
    bool                familiar = FALSE;

private:
	Player(const Player&);
	Player& operator=(const Player&);
};

/* mudding experience for newbies */
#define MEXP_TOTAL_NEWBIE	0
#define MEXP_LEGACY_NEWBIE	1
#define MEXP_LEGACY_OLDBIE	2

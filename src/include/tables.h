/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

struct helpfile_table_type
{
	String name;
	int	group;
};

struct MagT_type
{
	String male;
	String female;
};

struct CleT_type
{
	String male;
	String female;
};

struct ThiT_type
{
	String male;
	String female;
};

struct WarT_type
{
	String male;
	String female;
};

struct ftoa_type
{
	String alpha;
	long flag;
};

struct chan_type
{
	String name;
	String prefix_self;
	String other_str;
	int cslot;
	long bit;
	long revoke_bit;
};

struct revoke_type
{
	String name;
	long bit;
	String message;
};

#define CAND_CHAR	0
#define CAND_MOB	1
#define CAND_PLAYER	2
#define CAND_OBJ	3
#define	CAND_ROOM	4

#define FIELD_PLAYER	0
#define FIELD_Player	1
#define FIELD_CGROUP	2
#define FIELD_WIZNET	3
#define FIELD_REVOKE	4
#define	FIELD_CENSOR	5
#define	FIELD_COMM	6
#define	FIELD_ACT	7
#define	FIELD_OFF	8
#define	FIELD_FORM	9
#define	FIELD_PART	10
#define	FIELD_EXTRA	11
#define	FIELD_WEAR	12
#define	FIELD_WEAPON	13
#define	FIELD_ROOM	14

struct flag_type
{
    String name;
    unsigned int bit;
    bool settable;
};

struct field_type
{
	String			name;
	const std::vector<flag_type> &flag_table;
	int				cand;
	int				see_mob;
	int				see_plr;
	int				mod_mob;
	int				mod_plr;
};

struct position_type
{
    String name;
    String short_name;
};

struct sex_type
{
    String name;
};

struct size_type
{
    String name;
};

struct csetting_type
{
	int color;
	int bold;
	String name;
};

struct color_type
{
	String name;
	String code;
	int color;
	int bold;
};

// sector types - Montrey (2014)
struct sector_type {
    String name;
    sh_int type;
};

struct raffects
{
    String description;
    String shortname;
    int group;
    int id;
    int chance;
    long add;
};


/* game tables */
extern  const   std::vector<position_type> position_table;
extern  const   std::vector<sex_type>      sex_table;
extern  const   std::vector<size_type>     size_table;
extern  const   struct  raffects           raffects[MAX_RAFFECTS];
extern  const   String Msyl1[];
extern  const   String Msyl2[];
extern  const   String Msyl3[];
extern  const   String Fsyl1[];
extern  const   String Fsyl2[];
extern  const   String Fsyl3[];
extern  const   std::vector<MagT_type>     MagT_table;
extern  const   std::vector<CleT_type>     CleT_table;
extern  const   std::vector<ThiT_type>     ThiT_table;
extern  const   std::vector<WarT_type>     WarT_table;
extern	const	std::vector<chan_type>     chan_table;		/* new channel table */
extern	const	std::vector<revoke_type>   revoke_table;		/* revoke command table */
extern	const	std::vector<ftoa_type>     ftoa_table;		/* flags to alpha */
extern	const	std::vector<csetting_type> csetting_table;	/* color settings table */
extern	const	std::vector<color_type>    color_table;		/* color table */
extern	const	std::vector<helpfile_table_type> helpfile_table;	/* help name group table */
extern	const	std::vector<sector_type>   sector_table;

/* flag tables */
extern	const	String field_cand[];
extern	const	std::vector<field_type> flag_fields;
extern  const   std::vector<flag_type> plr_flags;
extern  const   std::vector<flag_type> act_flags;
extern  const   std::vector<flag_type> pcdata_flags;
extern  const   std::vector<flag_type> wiz_flags;
extern  const   std::vector<flag_type> affect_flags;
extern  const   std::vector<flag_type> off_flags;
extern  const   std::vector<flag_type> imm_flags;
extern  const   std::vector<flag_type> form_flags;
extern  const   std::vector<flag_type> part_flags;
extern  const   std::vector<flag_type> comm_flags;
extern  const   std::vector<flag_type> censor_flags;
extern  const   std::vector<flag_type> revoke_flags;
extern  const   std::vector<flag_type> extra_flags;
extern  const   std::vector<flag_type> wear_flags;
extern  const   std::vector<flag_type> weapon_flags;
extern  const   std::vector<flag_type> container_flags;
extern  const   std::vector<flag_type> portal_flags;
extern  const   std::vector<flag_type> room_flags;
extern  const   std::vector<flag_type> exit_flags;
extern  const   std::vector<flag_type> cgroup_flags;

long flag_lookup(const String& name, const std::vector<flag_type>& flag_table);

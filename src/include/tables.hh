#pragma once

#include "declare.hh"

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

struct chan_type
{
	String name;
	String prefix_self;
	String other_str;
	int cslot;
	Flags::Bit bit;
	Flags::Bit revoke_bit;
};

struct revoke_type
{
	String name;
	Flags::Bit bit;
	String message;
};

#define CAND_CHAR	0
#define CAND_MOB	1
#define CAND_PLAYER	2
#define CAND_OBJ	3
#define	CAND_ROOM	4

#define FIELD_PLAYER	0
#define FIELD_PCDATA	1
#define FIELD_CGROUP	2
#define FIELD_WIZNET	3
#define FIELD_REVOKE	4
#define	FIELD_CENSOR	5
#define	FIELD_COMM	6
#define	FIELD_ACT	7
#define	FIELD_OFF	8
#define	FIELD_FORM	9
#define	FIELD_PART	10
#define FIELD_GROUP 11
#define	FIELD_EXTRA	12
#define	FIELD_WEAR	13
#define	FIELD_WEAPON	14
#define	FIELD_ROOM	15

struct flag_type
{
    String name;
    Flags::Bit bit;
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
    Flags add;
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
extern	const	std::vector<csetting_type> csetting_table;	/* color settings table */
extern	const	std::vector<color_type>    color_table;		/* color table */
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
extern  const   std::vector<flag_type> group_flags;
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
extern  const   std::vector<flag_type> cont_flags;

int flag_index_lookup(const String& name, const std::vector<flag_type>& flag_table);

#include "merc.h"
#include "tables.h"

String item_name(int item_type)
{
	int type;

	for (type = 0; type < item_table.size(); type++)
		if (item_type == item_table[type].type)
			return item_table[type].name;

	return "none";
}

String weapon_name(int weapon_type)
{
	int type;

	for (type = 0; type < weapon_table.size(); type++)
		if (weapon_type == weapon_table[type].type)
			return weapon_table[type].name;

	return "exotic";
}

/*
 * Return ascii name of an item type.
 */
String item_type_name(Object *obj)
{
	switch (obj->item_type) {
	case ITEM_LIGHT:            return "light";

	case ITEM_SCROLL:           return "scroll";

	case ITEM_WAND:             return "wand";

	case ITEM_STAFF:            return "staff";

	case ITEM_WEAPON:           return "weapon";

	case ITEM_TREASURE:         return "treasure";

	case ITEM_ARMOR:            return "armor";

	case ITEM_CLOTHING:         return "clothing";

	case ITEM_POTION:           return "potion";

	case ITEM_FURNITURE:        return "furniture";

	case ITEM_TRASH:            return "trash";

	case ITEM_CONTAINER:        return "container";

	case ITEM_DRINK_CON:        return "drink container";

	case ITEM_KEY:              return "key";

	case ITEM_FOOD:             return "food";

	case ITEM_MONEY:            return "money";

	case ITEM_BOAT:             return "boat";

	case ITEM_CORPSE_NPC:       return "npc corpse";

	case ITEM_CORPSE_PC:        return "pc corpse";

	case ITEM_FOUNTAIN:         return "fountain";

	case ITEM_PILL:             return "pill";

	case ITEM_MAP:              return "map";

	case ITEM_PORTAL:           return "portal";

	case ITEM_WARP_STONE:       return "warp stone";

	case ITEM_GEM:              return "gem";

	case ITEM_JEWELRY:          return "jewelry";

	case ITEM_JUKEBOX:          return "juke box";

	case ITEM_MATERIAL:         return "material";

	case ITEM_ANVIL:            return "anvil";

//	case ITEM_COACH:            return "coach";

	case ITEM_WEDDINGRING:      return "weddingring";

	case ITEM_TOKEN:            return "token";
	}

	bug("Item_type_name: unknown type %d.", obj->item_type);
	return "(unknown)";
}

/*
 * Return ascii name of a damage type.
 */
String dam_type_name(int type)
{
	switch (type) {
		case DAM_NONE        : return "(none)";
		case DAM_CHARM       : return "charm";
		case DAM_BASH        : return "bash";
		case DAM_PIERCE      : return "pierce";
		case DAM_SLASH       : return "slash";
		case DAM_FIRE        : return "fire";
		case DAM_COLD        : return "cold";
		case DAM_ELECTRICITY : return "electricity";
		case DAM_ACID        : return "acid";
		case DAM_POISON      : return "poison";
		case DAM_NEGATIVE    : return "negative";
		case DAM_HOLY        : return "holy";
		case DAM_ENERGY      : return "energy";
		case DAM_MENTAL      : return "mental";
		case DAM_DISEASE     : return "disease";
		case DAM_DROWNING    : return "drowning";
		case DAM_LIGHT       : return "light";
		case DAM_SOUND       : return "sound";
	}

	bug("dam_type_name: unknown type %d.", type);
	return "(unknown)";
}


/*
 * Return ascii name of an affect location.
 */
String affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:            return "none";
	case APPLY_STR:             return "strength";
	case APPLY_DEX:             return "dexterity";
	case APPLY_INT:             return "intelligence";
	case APPLY_WIS:             return "wisdom";
	case APPLY_CON:             return "constitution";
	case APPLY_CHR:             return "charisma";
	case APPLY_SEX:             return "sex";
	case APPLY_AGE:             return "age";
	case APPLY_MANA:            return "mana";
	case APPLY_HIT:             return "hp";
	case APPLY_STAM:            return "stamina";
	case APPLY_GOLD:            return "gold";
	case APPLY_EXP:             return "experience";
	case APPLY_AC:              return "armor class";
	case APPLY_HITROLL:         return "hit roll";
	case APPLY_DAMROLL:         return "damage roll";
	case APPLY_SAVES:           return "saves";
	case APPLY_SAVING_ROD:      return "save vs rod";
	case APPLY_SAVING_PETRI:    return "save vs petrification";
	case APPLY_SAVING_BREATH:   return "save vs breath";
	case APPLY_SAVING_SPELL:    return "save vs spell";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(unknown)";
}

/*
 * Return ascii name of extra flags vector.
 */
String extra_bit_name(int extra_flags)
{
	String buf;

	if (extra_flags & ITEM_GLOW) buf += " glow";

	if (extra_flags & ITEM_HUM) buf += " hum";

	if (extra_flags & ITEM_COMPARTMENT) buf += " compartment";

	if (extra_flags & ITEM_TRANSPARENT) buf += " transparent";

	if (extra_flags & ITEM_EVIL) buf += " evil";

	if (extra_flags & ITEM_INVIS) buf += " invis";

	if (extra_flags & ITEM_MAGIC) buf += " magic";

	if (extra_flags & ITEM_NODROP) buf += " nodrop";

	if (extra_flags & ITEM_BLESS) buf += " bless";

	if (extra_flags & ITEM_ANTI_GOOD) buf += " anti-good";

	if (extra_flags & ITEM_ANTI_EVIL) buf += " anti-evil";

	if (extra_flags & ITEM_ANTI_NEUTRAL) buf += " anti-neutral";

	if (extra_flags & ITEM_NOREMOVE) buf += " noremove";

	if (extra_flags & ITEM_INVENTORY) buf += " inventory";

	if (extra_flags & ITEM_NOPURGE) buf += " nopurge";

	if (extra_flags & ITEM_VIS_DEATH) buf += " vis_death";

	if (extra_flags & ITEM_ROT_DEATH) buf += " rot_death";

	if (extra_flags & ITEM_NOLOCATE) buf += " no_locate";

	if (extra_flags & ITEM_SELL_EXTRACT) buf += " sell_extract";

	if (extra_flags & ITEM_BURN_PROOF) buf += " burn_proof";

	if (extra_flags & ITEM_NOSAC) buf += " no_sac";

	if (extra_flags & ITEM_NONMETAL) buf += " nonmetal";

	if (extra_flags & ITEM_MELT_DROP) buf += " melt_drop";

	if (extra_flags & ITEM_NOUNCURSE) buf += " no_uncurse";

	if (extra_flags & ITEM_QUESTSELL) buf += " quest_item";

	return buf.empty() ? "none" : buf.substr(1);
}

/* return ascii name of an act vector */
String act_bit_name(int act_flags, bool npc)
{
	String buf;

	if (npc) {
		buf += " npc";
		if (act_flags & ACT_NOSUMMON) buf += " no_summon";

		if (act_flags & ACT_SENTINEL) buf += " sentinel";

		if (act_flags & ACT_SCAVENGER) buf += " scavenger";

		if (act_flags & ACT_AGGRESSIVE) buf += " aggressive";

		if (act_flags & ACT_STAY_AREA) buf += " stay_area";

		if (act_flags & ACT_WIMPY) buf += " wimpy";

		if (act_flags & ACT_MORPH) buf += " morphed";

		if (act_flags & ACT_NOMORPH) buf += " nomorph";

		if (act_flags & ACT_STAY) buf += " stay";

		if (act_flags & ACT_OUTDOORS) buf += " outdoors";

		if (act_flags & ACT_INDOORS) buf += " indoors";

		if (act_flags & ACT_PET) buf += " pet";

		if (act_flags & ACT_TRAIN) buf += " train";

		if (act_flags & ACT_PRACTICE) buf += " practice";

		if (act_flags & ACT_UNDEAD) buf += " undead";

		if (act_flags & ACT_CLERIC) buf += " cleric";

		if (act_flags & ACT_MAGE) buf += " mage";

		if (act_flags & ACT_THIEF) buf += " thief";

		if (act_flags & ACT_WARRIOR) buf += " warrior";

		if (act_flags & ACT_NOALIGN) buf += " no_align";

		if (act_flags & ACT_NOPURGE) buf += " no_purge";

		if (act_flags & ACT_IS_HEALER) buf += " healer";

		if (act_flags & ACT_IS_CHANGER) buf += " changer";

		if (act_flags & ACT_GAIN) buf += " skill_train";

		if (act_flags & ACT_NOPUSH) buf += " nopush";
	}
	else {
		buf += " player";
		if (act_flags & PLR_NOSUMMON) buf += " no_summon";

		if (act_flags & PLR_AUTOASSIST) buf += " autoassist";

		if (act_flags & PLR_AUTOEXIT) buf += " autoexit";

		if (act_flags & PLR_AUTOLOOT) buf += " autoloot";

		if (act_flags & PLR_AUTOSAC) buf += " autosac";

		if (act_flags & PLR_AUTOGOLD) buf += " autogold";

		if (act_flags & PLR_AUTOSPLIT) buf += " autosplit";

		if (act_flags & PLR_TICKS) buf += " autotick";

		if (act_flags & PLR_CANLOOT) buf += " loot_corpse";

		if (act_flags & PLR_NOSUMMON) buf += " no_summon";

		if (act_flags & PLR_NOFOLLOW) buf += " no_follow";

		if (act_flags & PLR_FREEZE) buf += " frozen";

		if (act_flags & PLR_THIEF) buf += " thief";

		if (act_flags & PLR_KILLER) buf += " killer";

		if (act_flags & PLR_COLOR) buf += " color";

		if (act_flags & PLR_LOOKINPIT) buf += " lookinpit";

		if (act_flags & PLR_DEFENSIVE) buf += " showdef";

		if (act_flags & PLR_WIMPY) buf += " autorecall";

		if (act_flags & PLR_QUESTOR) buf += " questor";

		if (act_flags & PLR_COLOR2) buf += " crazycolor";

		if (act_flags & PLR_MAKEBAG) buf += " newbiehelp";

		if (act_flags & PLR_PERMIT) buf += " permit";

		if (act_flags & PLR_CLOSED) buf += " locker_closed";

		if (act_flags & PLR_LOG) buf += " logged";

		if (act_flags & PLR_NOPK) buf += " nopk";
	}

	return buf.empty() ? "none" : buf.substr(1);
}

String comm_bit_name(int comm_flags)
{
	String buf;

	if (comm_flags & COMM_QUIET) buf += " quiet";

	if (comm_flags & COMM_DEAF) buf += " deaf";

	if (comm_flags & COMM_NOWIZ) buf += " no_wiz";

	if (comm_flags & COMM_NOPRAY) buf += " no_pray";

	if (comm_flags & COMM_NOAUCTION) buf += " no_auction";

	if (comm_flags & COMM_NOGOSSIP) buf += " no_gossip";

	if (comm_flags & COMM_NOFLAME) buf += " no_flame";

	if (comm_flags & COMM_NOQUESTION) buf += " no_question";

	if (comm_flags & COMM_NOMUSIC) buf += " no_music";

	if (comm_flags & COMM_NOIC) buf += " no_ic";

	if (comm_flags & COMM_COMPACT) buf += " compact";

	if (comm_flags & COMM_BRIEF) buf += " brief";

	if (comm_flags & COMM_PROMPT) buf += " prompt";

	if (comm_flags & COMM_COMBINE) buf += " combine";

	if (comm_flags & COMM_NOCLAN) buf += " no_clantalk";

	if (comm_flags & COMM_SHOW_AFFECTS) buf += " show_affects";

	if (comm_flags & COMM_NOGRATS) buf += " no_grats";

	if (comm_flags & COMM_NOQUERY) buf += " no_query";

	if (comm_flags & COMM_NOPAGE) buf += " no_page";

	if (comm_flags & COMM_NOANNOUNCE) buf += " no_announce";

	if (comm_flags & COMM_NOSOCIAL) buf += " no_social";

	if (comm_flags & COMM_NOQWEST) buf += " no_qwest";

	if (comm_flags & COMM_AFK) buf += " afk";

	return buf.empty() ? "none" : buf.substr(1);
}

/* nochannel flags - by Xenith */
String revoke_bit_name(int revoke_flags)
{
	String buf;

	if (revoke_flags & REVOKE_NOCHANNELS) buf += " no_channels";

	if (revoke_flags & REVOKE_FLAMEONLY) buf += " flameonly";

	if (revoke_flags & REVOKE_GOSSIP) buf += " revoke_gossip";

	if (revoke_flags & REVOKE_FLAME) buf += " revoke_flame";

	if (revoke_flags & REVOKE_QWEST) buf += " revoke_qwest";

	if (revoke_flags & REVOKE_PRAY) buf += " revoke_pray";

	if (revoke_flags & REVOKE_AUCTION) buf += " revoke_auction";

	if (revoke_flags & REVOKE_CLAN) buf += " revoke_clan";

	if (revoke_flags & REVOKE_MUSIC) buf += " revoke_music";

	if (revoke_flags & REVOKE_QA) buf += " revoke_qa";

	if (revoke_flags & REVOKE_SOCIAL) buf += " revoke_social";

	if (revoke_flags & REVOKE_IC) buf += " revoke_ic";

	if (revoke_flags & REVOKE_GRATS) buf += " revoke_grats";

	if (revoke_flags & REVOKE_PAGE) buf += " revoke_page";

	if (revoke_flags & REVOKE_QTELL) buf += " revoke_qtell";

	if (revoke_flags & REVOKE_EMOTE) buf += " revoke_emote";

	if (revoke_flags & REVOKE_TELL) buf += " revoke_tell";

	if (revoke_flags & REVOKE_SAY) buf += " revoke_say";

	if (revoke_flags & REVOKE_EXP) buf += " revoke_exp";

	return buf.empty() ? "none" : buf.substr(1);
}

/* Command groups - Xenith */
String cgroup_bit_name(int flags)
{
	String buf;
	int i = 0;

	while (i < cgroup_flags.size()) {
		if (flags & cgroup_flags[i].bit) {
			buf += "";
			buf += cgroup_flags[i].name;
		}

		i++;
	}

	return buf.empty() ? "none" : buf.substr(1);
}

String censor_bit_name(int censor_flags)
{
	String buf;

	if (censor_flags & CENSOR_CHAN)         buf += " censor_channels";
	if (censor_flags & CENSOR_SPAM)         buf += " censor_spam";

	return buf.empty() ? "none" : buf.substr(1);
}

/* return ascii name of a plr vector */
String plr_bit_name(int plr_flags)
{
	String buf;

	if (plr_flags & PLR_OOC) buf += " ooc";

	if (plr_flags & PLR_CHATMODE) buf += " chatmode";

	if (plr_flags & PLR_PRIVATE) buf += " private";

	if (plr_flags & PLR_STOPCRASH) buf += " stopcrash";

	if (plr_flags & PLR_PK) buf += " pk";

	if (plr_flags & PLR_SHOWEMAIL) buf += " showemail";

	if (plr_flags & PLR_LINK_DEAD) buf += " linkdead";

	if (plr_flags & PLR_PAINT) buf += " paintbow";

	if (plr_flags & PLR_SNOOP_PROOF) buf += " snoopproof";

	if (plr_flags & PLR_NOSHOWLAST) buf += " noshowlast";

	if (plr_flags & PLR_AUTOPEEK) buf += " autopeek";

	if (plr_flags & PLR_HEEDNAME) buf += " heedname";

	if (plr_flags & PLR_SHOWLOST) buf += " showlost";

	if (plr_flags & PLR_SHOWRAFF) buf += " showraff";

	if (plr_flags & PLR_MARRIED) buf += " married";

	if (plr_flags & PLR_SQUESTOR) buf += " squestor";

	return buf.empty() ? "none" : buf.substr(1);
}

/* return ascii name of a room vector */
String room_bit_name(int flags)
{
	String buf;

	if (flags & ROOM_DARK) buf += " dark";

	if (flags & ROOM_NOLIGHT) buf += " nolight";

	if (flags & ROOM_NO_MOB) buf += " nomob";

	if (flags & ROOM_INDOORS) buf += " indoors";

	if (flags & ROOM_LOCKER) buf += " locker";

	if (flags & ROOM_FEMALE_ONLY) buf += " female";

	if (flags & ROOM_MALE_ONLY) buf += " male";

	if (flags & ROOM_NOSLEEP) buf += " nosleep";

	if (flags & ROOM_NOVISION) buf += " novision";

	if (flags & ROOM_PRIVATE) buf += " private";

	if (flags & ROOM_SAFE) buf += " safe";

	if (flags & ROOM_SOLITARY) buf += " solitary";

	if (flags & ROOM_PET_SHOP) buf += " petshop";

	if (flags & ROOM_NO_RECALL) buf += " norecall";

	if (flags & ROOM_IMP_ONLY) buf += " imp";

	if (flags & ROOM_GODS_ONLY) buf += " gods";

	if (flags & ROOM_HEROES_ONLY) buf += " hero";

	if (flags & ROOM_NEWBIES_ONLY) buf += " newbie";

	if (flags & ROOM_LAW) buf += " law";

	if (flags & ROOM_NOWHERE) buf += " nowhere";

	if (flags & ROOM_BANK) buf += " bank";

	if (flags & ROOM_LEADER_ONLY) buf += " leader";

	if (flags & ROOM_TELEPORT) buf += " teleport";

	if (flags & ROOM_UNDER_WATER) buf += " underwater";

	if (flags & ROOM_NOPORTAL) buf += " noportal";

	if (flags & ROOM_REMORT_ONLY) buf += " remort";

	if (flags & ROOM_NOQUEST) buf += " noquest";

	if (flags & ROOM_SILENT) buf += " silent";

	return buf.empty() ? "none" : buf.substr(1);
}

String wear_bit_name(int wear_flags)
{
	String buf;

	if (wear_flags & ITEM_TAKE) buf += " take";

	if (wear_flags & ITEM_WEAR_FINGER) buf += " finger";

	if (wear_flags & ITEM_WEAR_NECK) buf += " neck";

	if (wear_flags & ITEM_WEAR_BODY) buf += " torso";

	if (wear_flags & ITEM_WEAR_HEAD) buf += " head";

	if (wear_flags & ITEM_WEAR_LEGS) buf += " legs";

	if (wear_flags & ITEM_WEAR_FEET) buf += " feet";

	if (wear_flags & ITEM_WEAR_HANDS) buf += " hands";

	if (wear_flags & ITEM_WEAR_ARMS) buf += " arms";

	if (wear_flags & ITEM_WEAR_SHIELD) buf += " shield";

	if (wear_flags & ITEM_WEAR_ABOUT) buf += " body";

	if (wear_flags & ITEM_WEAR_WAIST) buf += " waist";

	if (wear_flags & ITEM_WEAR_WRIST) buf += " wrist";

	if (wear_flags & ITEM_WIELD) buf += " wield";

	if (wear_flags & ITEM_HOLD) buf += " hold";

	if (wear_flags & ITEM_WEAR_FLOAT) buf += " float";

	if (wear_flags & ITEM_WEAR_WEDDINGRING) buf += " weddingring";

	return buf.empty() ? "none" : buf.substr(1);
}

String form_bit_name(int form_flags)
{
	String buf;

	if (form_flags & FORM_POISON) buf += " poison";
	else if (form_flags & FORM_EDIBLE) buf += " edible";

	if (form_flags & FORM_MAGICAL) buf += " magical";

	if (form_flags & FORM_INSTANT_DECAY) buf += " instant_rot";

	if (form_flags & FORM_OTHER) buf += " other";

	if (form_flags & FORM_ANIMAL) buf += " animal";

	if (form_flags & FORM_SENTIENT) buf += " sentient";

	if (form_flags & FORM_UNDEAD) buf += " undead";

	if (form_flags & FORM_CONSTRUCT) buf += " construct";

	if (form_flags & FORM_MIST) buf += " mist";

	if (form_flags & FORM_INTANGIBLE) buf += " intangible";

	if (form_flags & FORM_BIPED) buf += " biped";

	if (form_flags & FORM_CENTAUR) buf += " centaur";

	if (form_flags & FORM_INSECT) buf += " insect";

	if (form_flags & FORM_SPIDER) buf += " spider";

	if (form_flags & FORM_CRUSTACEAN) buf += " crustacean";

	if (form_flags & FORM_WORM) buf += " worm";

	if (form_flags & FORM_BLOB) buf += " blob";

	if (form_flags & FORM_MAMMAL) buf += " mammal";

	if (form_flags & FORM_BIRD) buf += " bird";

	if (form_flags & FORM_REPTILE) buf += " reptile";

	if (form_flags & FORM_SNAKE) buf += " snake";

	if (form_flags & FORM_DRAGON) buf += " dragon";

	if (form_flags & FORM_AMPHIBIAN) buf += " amphibian";

	if (form_flags & FORM_FISH) buf += " fish";

	if (form_flags & FORM_COLD_BLOOD) buf += " cold_blooded";

	return buf.empty() ? "none" : buf.substr(1);
}

String part_bit_name(int part_flags)
{
	String buf;

	if (part_flags & PART_HEAD) buf += " head";

	if (part_flags & PART_ARMS) buf += " arms";

	if (part_flags & PART_LEGS) buf += " legs";

	if (part_flags & PART_HEART) buf += " heart";

	if (part_flags & PART_BRAINS) buf += " brains";

	if (part_flags & PART_GUTS) buf += " guts";

	if (part_flags & PART_HANDS) buf += " hands";

	if (part_flags & PART_FEET) buf += " feet";

	if (part_flags & PART_FINGERS) buf += " fingers";

	if (part_flags & PART_EAR) buf += " ears";

	if (part_flags & PART_EYE) buf += " eyes";

	if (part_flags & PART_LONG_TONGUE) buf += " long_tongue";

	if (part_flags & PART_EYESTALKS) buf += " eyestalks";

	if (part_flags & PART_TENTACLES) buf += " tentacles";

	if (part_flags & PART_FINS) buf += " fins";

	if (part_flags & PART_WINGS) buf += " wings";

	if (part_flags & PART_TAIL) buf += " tail";

	if (part_flags & PART_CLAWS) buf += " claws";

	if (part_flags & PART_FANGS) buf += " fangs";

	if (part_flags & PART_HORNS) buf += " horns";

	if (part_flags & PART_SCALES) buf += " scales";

	return buf.empty() ? "none" : buf.substr(1);
}

String wiz_bit_name(int wiz_flags)
{
	String buf;

	for (int flag = 0; flag < wiznet_table.size(); flag++)
		if (wiz_flags & wiznet_table[flag].flag) {
			buf += ' ';
			buf += wiznet_table[flag].name;
		}

	return buf.empty() ? "none" : buf.substr(1);
}

String weapon_bit_name(int weapon_flags)
{
	String buf;

	if (weapon_flags & WEAPON_FLAMING) buf += " flaming";

	if (weapon_flags & WEAPON_FROST) buf += " frost";

	if (weapon_flags & WEAPON_VAMPIRIC) buf += " vampiric";

	if (weapon_flags & WEAPON_SHARP) buf += " sharp";

	if (weapon_flags & WEAPON_VORPAL) buf += " vorpal";

	if (weapon_flags & WEAPON_TWO_HANDS) buf += " two-handed";

	if (weapon_flags & WEAPON_SHOCKING) buf += " shocking";

	if (weapon_flags & WEAPON_POISON) buf += " poison";

	return buf.empty() ? "none" : buf.substr(1);
}

String cont_bit_name(int cont_flags)
{
	String buf;

	if (cont_flags & CONT_CLOSEABLE) buf += " closable";

	if (cont_flags & CONT_PICKPROOF) buf += " pickproof";

	if (cont_flags & CONT_CLOSED) buf += " closed";

	if (cont_flags & CONT_LOCKED) buf += " locked";

	return buf.empty() ? "none" : buf.substr(1);
}

String off_bit_name(int off_flags)
{
	String buf;

	if (off_flags & OFF_AREA_ATTACK) buf += " area attack";

	if (off_flags & OFF_BACKSTAB) buf += " backstab";

	if (off_flags & OFF_BASH) buf += " bash";

	if (off_flags & OFF_BERSERK) buf += " berserk";

	if (off_flags & OFF_DISARM) buf += " disarm";

	if (off_flags & OFF_DODGE) buf += " dodge";

	if (off_flags & OFF_FAST) buf += " fast";

	if (off_flags & OFF_KICK) buf += " kick";

	if (off_flags & OFF_KICK_DIRT) buf += " kick_dirt";

	if (off_flags & OFF_PARRY) buf += " parry";

	if (off_flags & OFF_RESCUE) buf += " rescue";

	if (off_flags & OFF_TRIP) buf += " trip";

	if (off_flags & OFF_CRUSH) buf += " crush";

	if (off_flags & ASSIST_ALL) buf += " assist_all";

	if (off_flags & ASSIST_ALIGN) buf += " assist_align";

	if (off_flags & ASSIST_RACE) buf += " assist_race";

	if (off_flags & ASSIST_PLAYERS) buf += " assist_players";

	if (off_flags & ASSIST_GUARD) buf += " assist_guard";

	if (off_flags & ASSIST_VNUM) buf += " assist_vnum";

	return buf.empty() ? "none" : buf.substr(1);
}

String get_color_name(int color, int bold)
{
	int i;

	for (i = 0; i < color_table.size(); i++)
		if (color_table[i].color == color
		    && color_table[i].bold  == bold)
			return color_table[i].name;

	return "";
}

String get_color_code(int color, int bold)
{
	int i;

	for (i = 0; i < color_table.size(); i++)
		if (color_table[i].color == color
		    && color_table[i].bold  == bold)
			return color_table[i].code;

	return "";
}

String get_custom_color_name(Character *ch, int slot)
{
	int color, bold;

	if (!IS_NPC(ch) && ch->pcdata->color[slot] != 0) {
		color = ch->pcdata->color[slot];
		bold  = ch->pcdata->bold [slot];
	}
	else {
		color = csetting_table[slot].color;
		bold  = csetting_table[slot].bold;
	}

	return get_color_name(color, bold);
}

String get_custom_color_code(Character *ch, int slot)
{
	int color, bold;

	if (!IS_NPC(ch) && ch->pcdata->color[slot] != 0) {
		color = ch->pcdata->color[slot];
		bold  = ch->pcdata->bold [slot];
	}
	else {
		color = csetting_table[slot].color;
		bold  = csetting_table[slot].bold;
	}

	return get_color_code(color, bold);
}

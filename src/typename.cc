#include <vector>

#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "Object.hh"
#include "Player.hh"
#include "String.hh"
#include "tables.hh"

String item_name(int item_type)
{
	for (const auto& entry : item_table)
		if (item_type == entry.type)
			return entry.name;

	return "none";
}

String weapon_name(int weapon_type)
{
	for (const auto& entry : weapon_table)
		if (weapon_type == entry.type)
			return entry.name;

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
	case ITEM_WARP_CRYSTAL:     return "warp crystal";
	}

	Logging::bug("Item_type_name: unknown type %d.", obj->item_type);
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
		case DAM_WOOD        : return "wood";
		case DAM_SILVER      : return "silver";
		case DAM_IRON        : return "iron";
	}

	Logging::bug("dam_type_name: unknown type %d.", type);
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
	case APPLY_MANA:            return "maximum mana";
	case APPLY_HIT:             return "maximum hp";
	case APPLY_STAM:            return "maximum stamina";
	case APPLY_GOLD:            return "gold gained";
	case APPLY_EXP_PCT:         return "experience gained";
	case APPLY_AC:              return "armor class";
	case APPLY_HITROLL:         return "hit roll";
	case APPLY_DAMROLL:         return "damage roll";
	case APPLY_SAVES:           return "saves";
	case APPLY_SAVING_ROD:      return "save vs rod";
	case APPLY_SAVING_PETRI:    return "save vs petrification";
	case APPLY_SAVING_BREATH:   return "save vs breath";
	case APPLY_SAVING_SPELL:    return "save vs spell";
	case APPLY_QUESTPOINTS:     return "quest points gained";
	case APPLY_SKILLPOINTS:     return "skill points gained";
	case APPLY_MANA_COST_PCT:   return "spell mana cost";
	case APPLY_STAM_COST_PCT:   return "skill stamina cost";
	case APPLY_WPN_DAMAGE_PCT:  return "weapon damage dealt";
	case APPLY_SPELL_DAMAGE_PCT:return "spell damage dealt";
	case APPLY_VAMP_BONUS_PCT:  return "vampiric weapon leeching";
	}

	Logging::bug("Affect_location_name: unknown location %d.", location);
	return "(unknown)";
}

const String print_bit_names(const std::vector<flag_type> &flag_table, const Flags& flags) {
	String buf;

	for (flag_type flag: flag_table)
		if (flags.has(flag.bit)) {
			buf += " ";
			buf += flag.name;
		}

	return buf.empty() ? "none" : buf.substr(1);
}

/*
 * Return ascii name of extra flags vector.
 */
const String extra_bit_name(const Flags& flags) {
	return print_bit_names(extra_flags, flags);
}

const String act_bit_name(const Flags& flags, bool npc) {
	if (npc) return String("npc ") + print_bit_names(act_flags, flags);
	return String("player ") + print_bit_names(plr_flags, flags);
}

const String comm_bit_name(const Flags& flags) {
	return print_bit_names(comm_flags, flags);
}

const String revoke_bit_name(const Flags& flags) {
	return print_bit_names(revoke_flags, flags);
}

const String cgroup_bit_name(const Flags& flags) {
	return print_bit_names(cgroup_flags, flags);
}

const String censor_bit_name(const Flags& flags) {
	return print_bit_names(censor_flags, flags);
}

/* return ascii name of a plr vector */
const String plr_bit_name(const Flags& flags) {
	return print_bit_names(pcdata_flags, flags);
}

const String room_bit_name(const Flags& flags) {
	return print_bit_names(room_flags, flags);
}

const String wear_bit_name(const Flags& flags) {
	return print_bit_names(wear_flags, flags);
}

const String form_bit_name(const Flags& flags) {
	return print_bit_names(form_flags, flags);
}

const String part_bit_name(const Flags& flags) {
	return print_bit_names(part_flags, flags);
}

const String cont_bit_name(const Flags& flags) {
	return print_bit_names(cont_flags, flags);
}

const String off_bit_name(const Flags& flags) {
	return print_bit_names(off_flags, flags);
}

String wiz_bit_name(const Flags& flags)
{
	String buf;

	for (const auto& entry : wiznet_table)
		if (flags.has(entry.flag)) {
			buf += ' ';
			buf += entry.name;
		}

	return buf.empty() ? "none" : buf.substr(1);
}

String get_color_name(int color, int bold)
{
	for (const auto& entry : color_table)
		if (entry.color == color
		 && entry.bold == bold)
			return entry.name;

	return "";
}

String get_color_code(int color, int bold)
{
	for (const auto& entry : color_table)
		if (entry.color == color
		 && entry.bold == bold)
			return entry.code;

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

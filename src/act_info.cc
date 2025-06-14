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

#include <vector>
#include <sstream>
#include <algorithm> // min and max

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "DepartedPlayer.hh"
#include "Descriptor.hh"
#include "Duel.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "lootv2.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Note.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "skill/skill.hh"
#include "sql.hh"
#include "Social.hh"
#include "String.hh"
#include "tables.hh"
#include "typename.hh"
#include "Weather.hh"
#include "World.hh"
#include "gem/gem.hh"
#include "Location.hh"
#include "RoomPrototype.hh"

extern void     email_file    args((Character *ch, const char *file, const char *str));

char   *const   where_name      [] = {
	"<used as light>     ",
	"<worn on finger>    ",
	"<worn on finger>    ",
	"<worn around neck>  ",
	"<worn around neck>  ",
	"<worn on torso>     ",
	"<worn on head>      ",
	"<worn on legs>      ",
	"<worn on feet>      ",
	"<worn on hands>     ",
	"<worn on arms>      ",
	"<worn as shield>    ",
	"<worn about body>   ",
	"<worn about waist>  ",
	"<worn around wrist> ",
	"<worn around wrist> ",
	"<wielded>           ",
	"<held>              ",
	"<floating nearby>   ",
	"<secondary weapon>  ",
	"<wedding ring>      ",
};


/*
 * Local functions.
 */
void    show_list_to_char       args((Object *list, Character *ch, bool fShort, bool fShowNothing, bool insidecont));
void    show_char_to_char_0     args((Character *victim, Character *ch));
void    show_char_to_char_1     args((Character *victim, Character *ch));
void    show_char_to_char       args((Character *list, Character *ch));
bool    check_blind             args((Character *ch));
const String list_exits(Character *ch, bool fAuto);

String format_obj_to_char(Object *obj, Character *ch, bool fShort)
{
	String buf;
	int diff;

	if ((fShort && obj->short_descr.empty())
	    || obj->description.empty())
		return buf;

	if (obj->num_settings > 0) {
		buf += gem::get_short_string(obj);
		buf += " ";
	}

	/* Color additions by Lotus */
	if (!ch->is_npc()
	    && ((ch->pcdata->questobj > 0 && obj->pIndexData->vnum == ch->pcdata->questobj)
	        || (ch->pcdata->squestobj != nullptr && ch->pcdata->squestobj == obj)))
		buf += "{f{R[TARGET] {x";

	if (IS_OBJ_STAT(obj, ITEM_INVIS))
		buf += "{W(Invis) ";

	if (affect::exists_on_char(ch, affect::type::detect_evil)
	    && IS_OBJ_STAT(obj, ITEM_EVIL))
		buf += "{R(Red Aura) ";

	if (affect::exists_on_char(ch, affect::type::detect_good)
	    && IS_OBJ_STAT(obj, ITEM_BLESS))
		buf += "{B(Blue Aura) ";

	if (affect::exists_on_char(ch, affect::type::detect_magic)
	    && IS_OBJ_STAT(obj, ITEM_MAGIC))
		buf += "{G(Magical) ";

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		buf += "{Y(Glowing) ";

	if (IS_OBJ_STAT(obj, ITEM_HUM))
		buf += "{C(Humming) ";

	/* flags for temp weapon affects -- Elrac */
	if (obj->item_type == ITEM_WEAPON) {
		if (affect::exists_on_obj(obj, affect::type::weapon_acidic))    buf += "{G(Ac) ";
		if (affect::exists_on_obj(obj, affect::type::weapon_flaming))   buf += "{Y(Fl) ";
		if (affect::exists_on_obj(obj, affect::type::weapon_frost))     buf += "{C(Fr) ";
		if (affect::exists_on_obj(obj, affect::type::weapon_vampiric))  buf += "{P(Bl) ";
		if (affect::exists_on_obj(obj, affect::type::weapon_shocking))  buf += "{V(Sh) ";
		if (affect::exists_on_obj(obj, affect::type::poison))           buf += "{H(Po) ";
	}

	/* flags for temp weapon affects and dazzling light -- Elrac */
	if (affect::exists_on_obj(obj, affect::type::dazzling_light))
		buf += "{W{f(Dazzling){x ";

	if (obj->condition <= 9 && obj->condition >= 0)         buf += "{b(Ruined) ";

	if (obj->condition >= 10 && obj->condition <= 24)       buf += "{b(Broken) ";

	buf += "{x";

	if (fShort)
		buf += obj->short_descr;
	else
		buf += obj->description;

	buf += "{x";

	if (ch->act_flags.has(PLR_LOOKINPIT) && !ch->is_npc()) {
		diff = get_usable_level(ch) - obj->level;

		if (obj->level >= LEVEL_IMMORTAL)  buf += " {V(Immortal Item)";
		else if (diff < -50)                    buf += " {R(You wish)";
		else if (diff < -25)                    buf += " {R(In your dreams)";
		else if (diff < -10)                    buf += " {P(Powerful Item)";
		else if (diff <   0)                    buf += " {P(Almost there)";
		else if (diff <  10)                    buf += " {G(Just your size)";
		else if (diff <  25)                    buf += " {G(Still useful)";
		else if (diff <  50)                    buf += " {H(Weak Item)";
		else                                    buf += " {H(Tis but a toy)";

		buf += "{x";
	}

	return buf;
}

void show_affect_to_char(const affect::Affect *paf, Character *ch)
{
	String buf;

	if (paf->type > affect::type::none)
		buf = Format::format("Effect '%s'", affect::lookup(paf->type).name);

	if (paf->location != 0 && paf->modifier != 0) {
		if (paf->where == TO_DEFENSE)
			buf += Format::format("%sodifies defense against %s by %d",
				buf.empty() ? "M" : " m", dam_type_name(paf->location), paf->modifier);
		else {
			buf += Format::format("%sffects %s by %d",
				buf.empty() ? "A" : " a",
				affect_loc_name(paf->location),
				paf->modifier);

			if (paf->location == APPLY_EXP_PCT
			 || paf->location == APPLY_MANA_COST_PCT
			 || paf->location == APPLY_STAM_COST_PCT
			 || paf->location == APPLY_WPN_DAMAGE_PCT
			 || paf->location == APPLY_SPELL_DAMAGE_PCT
			 || paf->location == APPLY_VAMP_BONUS_PCT
			 || paf->location == APPLY_FORGE_UNIQUE
			 || paf->location == APPLY_PRIESTESS_UNIQUE
			 || paf->location == APPLY_BREW_UNIQUE
			 || paf->location == APPLY_SCRIBE_UNIQUE
			 || paf->location == APPLY_TANK_UNIQUE
			 || paf->location == APPLY_NECRO_PIERCE_UNIQUE
			 || paf->location == APPLY_LIDDA_AURA_UNIQUE)
				buf += "%";
		}
	}

	if (IS_IMMORTAL(ch))
		buf += Format::format(", level %d", paf->level);

	if (paf->duration > -1)
		buf += Format::format(", %d hours", paf->duration);

	buf += ".";

	if (!paf->bitvector().empty()) {
		int num_flags = 0;

		for (int i = 0; i < 32; i++)
			if (paf->bitvector().has(static_cast<Flags::Bit>(1 << i)))
				num_flags++;

		switch (paf->where) {
		case TO_OBJECT:
			buf += Format::format(" Adds %s object flag%s.",
				extra_bit_name(paf->bitvector()), num_flags > 1 ? "s" : "");
			break;
/*
		case TO_WEAPON:
			buf += Format::format(" Adds %s weapon flag%s.",
				weapon_bit_name(paf->bitvector()), num_flags > 1 ? "s" : "");
			break;
*/
		}
	}

	buf += "\n";
	stc(buf, ch);
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(Object *list, Character *ch, bool fShort, bool fShowNothing, bool insidecont)
{
	Object *obj;
//	String output;
	char buf[MAX_STRING_LENGTH];
	int nShow = 0, iShow, count = 0;
	bool fCombine, foundcont = false;

	if (ch->desc == nullptr)
		return;

	String output;

	for (obj = list; obj != nullptr; obj = obj->next_content)
		count++;

	if (count > 8000) {
		stc("There are too many items to display.\n", ch);
		return;
	}

	// gross temporary fix
	String prgpstrShow[8000];
	int prgnShow[8000];

	/* Format the list of objects */
	for (obj = list; obj != nullptr; obj = obj->next_content) {
		if (!can_see_obj(ch, obj))
			continue;

		if (insidecont && obj->contains)
			foundcont = true;

		if (obj->wear_loc == WEAR_NONE) {
			String pstrShow = format_obj_to_char(obj, ch, fShort);
			fCombine = false;

			if (ch->is_npc() || ch->comm_flags.has(COMM_COMBINE)) {
				/* Look for duplicates, case sensitive.
				   Matches tend to be near end so run loop backwords. */
				for (iShow = nShow - 1; iShow >= 0; iShow--)
					if (!strcmp(prgpstrShow[iShow], pstrShow)) {
						prgnShow[iShow]++;
						fCombine = true;
						break;
					}
			}

			/* Couldn't combine, or didn't want to */
			if (!fCombine) {
				prgpstrShow [nShow] = pstrShow;
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/* Output the formatted list */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow].empty()) {
			continue;
		}

		if (ch->is_npc() || ch->comm_flags.has(COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) {
				Format::sprintf(buf, "(%2d) ", prgnShow[iShow]);
				output += buf;
			}
			else
				output += "     ";
		}

		output += prgpstrShow[iShow];
		output += '\n';

		if (output.size() > 15500) {
			if (ch->comm_flags.has(COMM_COMBINE))
				output += "     (More stuff not shown)\n";
			else
				output += "     (More stuff not shown -- COMBINE would help)\n";

			break;
		}
	}

	if (fShowNothing && nShow == 0) {
		if (ch->is_npc() || ch->comm_flags.has(COMM_COMBINE))
			stc("     ", ch);

		stc("Nothing.\n", ch);
	}

	page_to_char(output, ch);

	if (foundcont)  /* for evolved peek, list items in containers, but not twice nested */
		for (obj = list; obj != nullptr; obj = obj->next_content)
			if (can_see_obj(ch, obj) && obj->contains) {
				ptc(ch, "\n  Inside %s{x:\n", obj->short_descr);
				show_list_to_char(obj->contains, ch, true, true, false);
			}
} /* end show_list_to_char() */

void do_bonus(Character *ch, String argument) {
	String outbuf, tempbuf, blankbuff;
	bool found;
	
	outbuf +=
	"  ,                                                                  ,\n"
	" '`,                                                                '`,\n"
	" `,                                                                 `,\n"
	" .:.                                                                .:.\n"
	" )X(                                                                )X(\n"
	"[%%%]==============================================================[%%%]\n";

	found = false;
	tempbuf =
	" |#| You are affected by the following bonuses:                     |#|\n"
	" |#|----------------------------------------------------------------|#|\n";

	for (int index = 1; index < MAX_ATTR; index++) {
		// skip these
		if (index == 7
		 || index == 8
		 || index == 10
		 || index == 11
		 || index == 25)
			continue;

		if ((GET_ATTR_MOD(ch, index)) == 0)
			continue;

		found = true;

		tempbuf += Format::format(" |#| %-30s                          %5d",
			affect_loc_name(index), GET_ATTR_MOD(ch, index));

		if (index == APPLY_EXP_PCT
		 || index == APPLY_MANA_COST_PCT
		 || index == APPLY_STAM_COST_PCT
		 || index == APPLY_WPN_DAMAGE_PCT
		 || index == APPLY_SPELL_DAMAGE_PCT
		 || index == APPLY_VAMP_BONUS_PCT
		 || index == APPLY_FORGE_UNIQUE
		 || index == APPLY_PRIESTESS_UNIQUE
		 || index == APPLY_BREW_UNIQUE
	     || index == APPLY_SCRIBE_UNIQUE
		 || index == APPLY_TANK_UNIQUE
		 || index == APPLY_NECRO_PIERCE_UNIQUE
		 || index == APPLY_LIDDA_AURA_UNIQUE)
			tempbuf += "%";
		else
			tempbuf += " ";

		tempbuf += " |#|\n";
	}

	if (found)
		outbuf += tempbuf;

	if (found)
		tempbuf = 
		" |#|----------------------------------------------------------------|#|\n";

	
	found = false;
	tempbuf =
	" |#|----------------------------------------------------------------|#|\n"
	" |#| You are affected by the following set bonuses:                 |#|\n"
	" |#|----------------------------------------------------------------|#|\n";
	
	unsigned int found_set = 0;
	Object *obj;
	int iWear;
	bool name_displayed, set2, set3, set4, set5 = false;
	blankbuff = " ";
  
	for (iWear = 0; iWear < MAX_WEAR; iWear++) { //loop thru once for each piece worn
		if ((obj = get_eq_char(ch, iWear)) != nullptr) {//if char is wearing something continue
			for (const affect::Affect *paf = affect::list_obj(obj); paf != nullptr; paf = paf->next) {
				if (paf->location == 0) //check if obj has an attr
					continue;			//doesn't so continue
				
				if (paf->location  >= 43 &&	//set gear is between 43 and 63
					(paf->location <= 63)){	//so only worry about that range
					
					found_set = ( paf->location - 42 );			//save the slot found
					found = true;
				
					if ((!name_displayed) && (GET_ATTR(ch, paf->location) >= 2)){
						tempbuf += Format::format(
						" |#| %-30s                          %6s", 
						set_table[found_set - 1].display,
						blankbuff);
						name_displayed = true;
						tempbuf += " |#|\n";
					}
					if (GET_ATTR(ch, paf->location) >= 2 && (!set2)){
						tempbuf += Format::format(" |#| %-45s  		  %1s", set_table[found_set - 1].set2,
																			blankbuff);
						set2 = true;
						tempbuf += " |#|\n";
					}
					if (GET_ATTR(ch, paf->location) >= 3 && (!set3)){
						tempbuf += Format::format(" |#| %-45s  		%3s", set_table[found_set - 1].set3,
																			blankbuff);
						set3 = true;
						tempbuf += " |#|\n";
					}
					if (GET_ATTR(ch, paf->location) >= 4 && (!set4)){
						tempbuf += Format::format(" |#| %-45s  		%3s", set_table[found_set - 1].set4,
																			blankbuff);
						set4 = true;
						tempbuf += " |#|\n";
					}
					if (GET_ATTR(ch, paf->location) >= 5 && (!set5)){
						tempbuf += Format::format(" |#| %-45s  		%3s", set_table[found_set - 1].set5,
																			blankbuff);
						set5 = true;
						tempbuf += " |#|\n";
					}
				}
			}
		}
	}
	
	if (found)
		outbuf += tempbuf;

	if (found)
		tempbuf = " |#|----------------------------------------------------------------|#|\n";
	
	
	found = false;
	tempbuf +=
	" |#| Your defense against damage is modified by:                    |#|\n"
	" |#|----------------------------------------------------------------|#|\n";

	for (int index = 1; index <= DAM_IRON; index++) {
		// skip these
		if (index == 16
		 || index == 17
		 || index == 20)
			continue;

		if ((GET_DEFENSE_MOD(ch, index)) == 0)
			continue;

		found = true;

		tempbuf += Format::format(" |#| %-30s                          %5d%%",
			dam_type_name(index), GET_DEFENSE_MOD(ch, index));

		tempbuf += " |#|\n";
	}

	if (found)
		outbuf += tempbuf;

	outbuf += 
	" |#|================================================================|#|\n"
	" |#|                                                                |#|\n"
	"[[|]]                                                              [[|]]\n";

	new_color(ch, CSLOT_SCORE_BORDER);
	page_to_char(outbuf, ch);
	set_color(ch, WHITE, NOBOLD);
}

void do_peek(Character *ch, String argument)
{
	Character *victim;
	bool all = false;

	if (ch->is_npc() || !get_skill_level(ch, skill::type::peek)) {
		stc("You are not skilled at peeking.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Peek at whom?\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ((victim = get_char_here(ch, arg1, VIS_CHAR)) == nullptr) {
		stc("You see no one by that name in the room.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Are you that paranoid?!?\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		stc("They probably wouldn't like that.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::peek))
		return;

	if (!roll_chance(get_skill_level(ch, skill::type::peek))) {
		stc("You can't seem to find a good angle...\n", ch);
		return;
	}

	if (!arg2.empty() && get_evolution(ch, skill::type::peek) > 1) {
		if (arg2 == "lore") {
			Object *obj;

			if (!get_skill_level(ch, skill::type::lore)) {
				stc("You aren't trained in the lore of items.\n", ch);
				return;
			}

			if (argument.empty()) {
				stc("What item in their inventory do you want to lore?\n", ch);
				return;
			}

			if ((obj = get_obj_carry(victim, argument)) == nullptr) {
				stc("They aren't carrying that.\n", ch);
				return;
			}

			WAIT_STATE(ch, skill::lookup(skill::type::lore).beats);

			if (!ch->is_npc() && !roll_chance(get_skill_level(ch, skill::type::lore)))
				act("You look at $p, but you can't find out any additional information.",
				    ch, obj, nullptr, TO_CHAR);
			else {
				spell_identify(skill::type::lore, (4 * obj->level) / 3, ch, obj,
				               TARGET_OBJ, get_evolution(ch, skill::type::lore));
				check_improve(ch, skill::type::lore, true, 4);
			}

			return;
		}
		else if (arg2 == "all" && get_evolution(ch, skill::type::peek) > 2)
			all = true;
	}

	act("You peek at $S inventory:", ch, nullptr, victim, TO_CHAR);
	show_list_to_char(victim->carrying, ch, true, true, all);
	check_improve(ch, skill::type::peek, true, 4);
}

void show_char_to_char_0(Character *victim, Character *ch)
{
	String buf;
	char message[MAX_STRING_LENGTH];

	/* Pretty little link dead by Demonfire */
	if (!victim->is_npc()) {
		if (victim->pcdata->plr_flags.has(PLR_LINK_DEAD))
			buf += "{G(LinkDead) ";
	}

	if (victim->is_npc() && !ch->is_npc()
	    && ((ch->pcdata->questmob > 0 && victim->pIndexData->vnum == ch->pcdata->questmob)
	        || (!ch->desc->original && ch->pcdata->squestmob != nullptr && victim == ch->pcdata->squestmob)))
		buf += "{f{R[TARGET] {x";

	if (victim->comm_flags.has(COMM_AFK))
		buf += "{b[AFK] ";

	if (affect::exists_on_char(victim, affect::type::invis))
		buf += "{C(Invis) ";

	if (affect::exists_on_char(victim, affect::type::midnight))
		buf += "{c(Shadowy) ";

	if (affect::exists_on_char(victim, affect::type::hex))
		buf += "{c(Dark Aura) ";

	if (victim->invis_level)
		buf += "{T(Wizi) ";

	if (victim->lurk_level)
		buf += "{H(Lurk) ";

	if (!victim->is_npc() && !victim->pcdata->aura.empty()) {
		char string[MAX_INPUT_LENGTH];
		Format::sprintf(string, "{W(%s{W) ", victim->pcdata->aura);
		buf += string;
	}

	if (affect::exists_on_char(victim, affect::type::hide)) buf += "{B(Hide) ";

	if (affect::exists_on_char(victim, affect::type::charm_person)) buf += "{M(Charmed) ";

	if (affect::exists_on_char(victim, affect::type::pass_door)) buf += "{c(Translucent) ";

	if (affect::exists_on_char(victim, affect::type::faerie_fire)) buf += "{P(Pink Aura) ";

	if (affect::exists_on_char(victim, affect::type::flameshield)) buf += "{b(Flaming Aura) ";

	if (IS_EVIL(victim) && affect::exists_on_char(ch, affect::type::detect_evil)) buf += "{R(Red Aura) ";

	if (IS_GOOD(victim) && affect::exists_on_char(ch, affect::type::detect_good)) buf += "{Y(Golden Aura) ";

	if (affect::exists_on_char(victim, affect::type::sanctuary)) buf += "{W(White Aura) ";

	if (!victim->is_npc() && victim->act_flags.has(PLR_KILLER)) buf += "{R(KILLER) ";

	if (!victim->is_npc() && victim->act_flags.has(PLR_THIEF)) buf += "{B(THIEF) ";

	buf += "{x";

	if (get_position(victim) == victim->start_pos && !victim->long_descr.empty()) {
		if (victim->is_npc())
			new_color(ch, CSLOT_MISC_MOBILES);
		else
			new_color(ch, CSLOT_MISC_PLAYERS);

		buf += victim->long_descr;
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* took PERS out of this line, with both show_char_to_char and PERS checking can_see,
	   means that sometimes it shows players (and mobs) as 'someone' in the room list -- Montrey */
	buf += (victim->is_npc() ? victim->short_descr : victim->name);

	if (!victim->is_npc() && !ch->comm_flags.has(COMM_BRIEF)
	    &&   get_position(victim) >= POS_STANDING && ch->on == nullptr) {
		new_color(ch, CSLOT_MISC_PLAYERS);
		buf += victim->pcdata->title;
		stc("{x{a", ch);
		set_color(ch, WHITE, NOBOLD);
	}

	switch (get_position(victim)) {
	case POS_DEAD:     buf += " is DEAD!!";              break;

	case POS_MORTAL:   buf += " is mortally wounded.";   break;

	case POS_INCAP:    buf += " is incapacitated.";      break;

	case POS_STUNNED:  buf += " is lying here stunned."; break;

	case POS_SLEEPING:
		if (victim->on != nullptr) {
			if (victim->on->value[2].flags().has(SLEEP_AT)) {
				Format::sprintf(message, " is sleeping at %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else if (victim->on->value[2].flags().has(SLEEP_ON)) {
				Format::sprintf(message, " is sleeping on %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else {
				Format::sprintf(message, " is sleeping in %s.",
				        victim->on->short_descr);
				buf += message;
			}
		}
		else
			buf += " is sleeping here.";

		break;

	case POS_RESTING:
		if (victim->on != nullptr) {
			if (victim->on->value[2].flags().has(REST_AT)) {
				Format::sprintf(message, " is resting at %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else if (victim->on->value[2].flags().has(REST_ON)) {
				Format::sprintf(message, " is resting on %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else {
				Format::sprintf(message, " is resting in %s.",
				        victim->on->short_descr);
				buf += message;
			}
		}
		else
			buf += " is resting here.";

		break;

	case POS_SITTING:
		if (victim->on != nullptr) {
			if (victim->on->value[2].flags().has(SIT_AT)) {
				Format::sprintf(message, " is sitting at %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else if (victim->on->value[2].flags().has(SIT_ON)) {
				Format::sprintf(message, " is sitting on %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else {
				Format::sprintf(message, " is sitting in %s.",
				        victim->on->short_descr);
				buf += message;
			}
		}
		else
			buf += " is sitting here.";

		break;

	case POS_STANDING:
		if (victim->on != nullptr) {
			if (victim->on->value[2].flags().has(STAND_AT)) {
				Format::sprintf(message, " is standing at %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else if (victim->on->value[2].flags().has(STAND_ON)) {
				Format::sprintf(message, " is standing on %s.",
				        victim->on->short_descr);
				buf += message;
			}
			else {
				Format::sprintf(message, " is standing in %s.",
				        victim->on->short_descr);
				buf += message;
			}
		}
		else
			buf += " is here.";

		break;

	case POS_FLYING:
		buf += " is flying here.";
		break;

	case POS_FIGHTING:
		buf += " is here, fighting ";

		if (victim->fighting == nullptr)
			buf += "thin air??";
		else if (victim->fighting == ch)
			buf += "YOU! DOH!";
		else if (victim->in_room == victim->fighting->in_room) {
			buf += PERS(victim->fighting, ch, VIS_CHAR);
			buf += ".";
		}
		else
			buf += "with shadow-boxers??";

		break;
	}

	buf += "\n";
	buf[0] = toupper(buf[0]);
	new_color(ch, victim->is_npc() ? CSLOT_MISC_MOBILES : CSLOT_MISC_PLAYERS);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void show_char_to_char_1(Character *victim, Character *ch)
{
	String buf;
	Object *obj;
	int iWear;
	int percent;
	bool found;

	if (can_see_char(victim, ch)) {
		if (ch == victim)
			act("$n looks at $mself.", ch, nullptr, nullptr, TO_ROOM);
		else {
			act("$n looks at you.", ch, nullptr, victim, TO_VICT);
			act("$n looks at $N.",  ch, nullptr, victim, TO_NOTVICT);
		}
	}

	if (!victim->description.empty()) {
		set_color(ch, PURPLE, NOBOLD);
		stc(victim->description, ch);
		set_color(ch, WHITE, NOBOLD);
	}
	else {
		set_color(ch, PURPLE, NOBOLD);
		act("Nothing Special.", ch, nullptr, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
	}

	if (GET_MAX_HIT(victim) > 0)
		percent = (100 * victim->hit) / GET_MAX_HIT(victim);
	else
		percent = -1;

	buf = PERS(victim, ch, VIS_CHAR);

	if (percent >= 100)
		buf += " is in excellent condition.\n";
	else if (percent >= 90)
		buf += " has a few scratches.\n";
	else if (percent >= 75)
		buf += " has some small wounds and bruises.\n";
	else if (percent >=  50)
		buf += " has quite a few wounds.\n";
	else if (percent >= 30)
		buf += " has some big nasty wounds and scratches.\n";
	else if (percent >= 15)
		buf += " looks pretty hurt.\n";
	else if (percent >= 1)
		buf += " is in awful condition.\n";
	else if (percent >= 0)
		buf += " will soon be toast!!!\n";
	else
		buf += " is in need of ***SERIOUS*** medical attention!!!\n";

	buf[0] = toupper(buf[0]);
	set_color(ch, CYAN, NOBOLD);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	found = false;

	if (!victim->is_npc()) {
		set_color(ch, RED, BOLD);
		Format::sprintf(buf, "%s has killed %d players and has been killed by %d players.\n",
		        GET_ATTR_SEX(victim) == SEX_NEUTRAL ? "It" : GET_ATTR_SEX(victim) == SEX_MALE ? "He" : "She"
		        , victim->pcdata->pckills, victim->pcdata->pckilled);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!victim->is_npc()) {
		set_color(ch, PURPLE, BOLD);
		Format::sprintf(buf, "%s has killed %d players in the arena and has been defeated by %d players in the arena.\n",
		        GET_ATTR_SEX(victim) == SEX_NEUTRAL ? "It" : GET_ATTR_SEX(victim) == SEX_MALE ? "He" : "She"
		        , victim->pcdata->arenakills, victim->pcdata->arenakilled);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!victim->is_npc() && !IS_IMMORTAL(victim)) {
		Format::sprintf(buf, "%s is a devout follower of %s.\n", victim->name,
		        !victim->pcdata->deity.empty() ? victim->pcdata->deity : "no one");
		set_color(ch, CYAN, BOLD);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!victim->is_npc()) {
		set_color(ch, BLUE, BOLD);
		Format::sprintf(buf, "%s is about %d years old.\n",
		        GET_ATTR_SEX(victim) == SEX_NEUTRAL ? "It" : GET_ATTR_SEX(victim) == SEX_MALE ? "He" : "She"
		        , GET_ATTR_AGE(victim));
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	for (iWear = 0; iWear < MAX_WEAR; iWear++) {
		if (!victim->is_npc() && !victim->pcdata->spouse.empty() && iWear == WEAR_WEDDINGRING
		    && !IS_IMMORTAL(ch)) /* so imms can see weddingrings on unmarried ppl */
			continue;

		if ((obj = get_eq_char(victim, iWear)) != nullptr
		    &&   can_see_obj(ch, obj)) {
			set_color(ch, GREEN, NOBOLD);

			if (!found) {
				stc("\n", ch);
				act("$N is using:", ch, nullptr, victim, TO_CHAR);
				found = true;
			}

			stc(where_name[iWear], ch);
			stc(format_obj_to_char(obj, ch, true), ch);
			stc("\n", ch);
			set_color(ch, WHITE, NOBOLD);
		}
	}

	if (victim != ch
	    &&   !ch->is_npc()
	    &&   ch->pcdata->plr_flags.has(PLR_AUTOPEEK)
	    && (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	    &&   number_percent() < get_skill_level(ch, skill::type::peek)) {
		set_color(ch, YELLOW, NOBOLD);
		stc("\nYou peek at the inventory:\n", ch);
		show_list_to_char(victim->carrying, ch, true, true, false);
		check_improve(ch, skill::type::peek, true, 4);
		/* Don't forget to deduct stamina for peeking. -- Outsider */
		deduct_stamina(ch, skill::type::peek);
	}

	set_color(ch, WHITE, NOBOLD);
	return;
}

void show_char_to_char(Character *list, Character *ch)
{
	Character *rch;

	for (rch = list; rch != nullptr; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		if (can_see_char(ch, rch))
			show_char_to_char_0(rch, ch);
		else if (ch->in_room->is_dark()
		         && !ch->in_room->is_very_dark()
		         && affect::exists_on_char(rch, affect::type::night_vision))
			stc("You see glowing eyes watching YOU!\n", ch);
	}
}

bool check_blind(Character *ch)
{
	if (IS_IMMORTAL(ch))
		return true;

	if (is_blinded(ch)) {
		stc("You can't see a thing!\n", ch);
		return false;
	}

	return true;
}

/*
 * the first number bold = 0,1 gives you dim or BRIGHT colors
 * the second number determines the colors as follows:
 * 30 = grey, 31 = red, 32 = green, 33 = yellow, 34 = blue, 35 = purple
 * 36 = cyan, 37 = white
 * special notes: on a MAC the colors may be screwy, 0,30 gives you black
 * 0,34 seems to be REALLY dark so it doesn't show up too well...
 */
void set_color(Character *ch, int color, int bold)
{
	char buf[MAX_INPUT_LENGTH];

	if (ch->act_flags.has(PLR_COLOR)) {
		Format::sprintf(buf, "\033[%d;%dm", bold, color);
		stc(buf, ch);
	}

	if (!ch->is_npc()) {
		ch->pcdata->lastcolor[0] = color;
		ch->pcdata->lastcolor[1] = bold;
	}
}

void new_color(Character *ch, int slot)
{
	if (!ch->is_npc() && ch->pcdata->color[slot] != 0)
		set_color(ch, ch->pcdata->color[slot], ch->pcdata->bold[slot]);
	else
		set_color(ch, csetting_table[slot].color, csetting_table[slot].bold);
}

/* changes your scroll */
void do_scroll(Character *ch, String argument)
{
	char buf[100];
	int lines;

	if (argument.empty()) {
		Format::sprintf(buf, "You currently display %d lines per page.\n",
		        ch->lines);
		stc(buf, ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_number()) {
		stc("You must provide a number.\n", ch);
		return;
	}

	lines = atoi(arg);

	if (lines < 10 || lines > 100) {
		stc("You must provide a reasonable number.\n", ch);
		return;
	}

	Format::sprintf(buf, "Scroll set to %d lines.\n", lines);
	stc(buf, ch);
	ch->lines = lines;
} /* end set_scroll() */

void do_socials(Character *ch, String argument)
{
	String dbuf;
	char buf[MAX_STRING_LENGTH];
	Social *iterator;
	int col;
	col = 0;

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		Format::sprintf(buf, "%-12s", iterator->name);
		dbuf += buf;

		if (++col % 6 == 0)
			dbuf += "\n";
	}

	if (col % 6 != 0)
		dbuf += "\n";

	page_to_char(dbuf, ch);
} /* end do_socials() */

/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_imotd(Character *ch, String argument)
{
	help(ch, "imotd");
}

void do_rules(Character *ch, String argument)
{
	help(ch, "rules");
}

void do_story(Character *ch, String argument)
{
	help(ch, "story");
}

void do_departed(Character *ch, String argument)
{
	DepartedPlayer *iterator;
	char name_buf[20];
	int i = 0;
	stc("\n", ch);
	stc(departed_list_line, ch);

	for (iterator = departed_list_head->next; iterator != departed_list_tail; iterator = iterator->next) {
		Format::sprintf(name_buf, "%-18s", iterator->name);
		stc(name_buf, ch);
		i++;

		if (i % 2 == 0) {
			iterator = iterator->next;

			if (iterator == departed_list_tail) {
				stc("\n", ch);
				i = 0;
				break;
			}

			stc(iterator->name, ch);
			stc("\n", ch);
			stc(departed_list_line, ch);
		}
	}

	if (i != 0)
		stc("\n", ch);
}

void do_wizlist(Character *ch, String argument)
{
	help(ch, "wizlist");
}

/* Alphabetizing stuff. Fun. */
int compare_clans(const void *p1, const void *p2)
{
	return strcmp((*(Clan **)p1)->name, (*(Clan **)p2)->name);
}

void do_clanlist(Character *ch, String argument)
{
	char lblock[MIL], rblock[MIL];
	Clan *clan;
	String buffer;
	int count = 0, l = 0, r = 0, i = 0, highremort = 0, highlevel = 0;
	struct clan_clan {
		char    name[20];
		char    title[150];
		char    rank[20];
		int  level;
		int  remort;
		int  status;
		bool    printed;
	};
	struct clan_clan clan_list[500];

	if (argument.empty()) {
		Clan *aClan[count_clans()], zero_clan;
		int index = 0;
		zero_clan.name = "zzzzzzzz";
		count = count_clans();

		while (index < count) {
			aClan[index] = &zero_clan;
			index++;
		}

		clan = clan_table_head->next;
		count = 0;

		while (clan != clan_table_tail) {
			if (clan == nullptr)
				continue;

			if (!clan->independent)
				aClan[count] = clan;

			clan = clan->next;
			count++;
		}

		qsort(aClan, count, sizeof(aClan[0]), compare_clans);
		count = 0;
		set_color(ch, CYAN, BOLD);
		stc("Current Clans of Legacy:\n\n", ch);
		stc("Clan Name                         Mail Name      Logo     Creator\n", ch);
		stc("----------------------------------------------------------------------\n", ch);
		set_color(ch, CYAN, NOBOLD);

		while (strcmp(aClan[count]->name, "zzzzzzzz")) {
			ptc(ch, "%-34s{x%-14s{x%-9s{x %s{x\n",
			    aClan[count]->clanname,
			    aClan[count]->name,
			    aClan[count]->who_name,
			    aClan[count]->creator);
			count++;
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* argument given: list clan membership */
	if (!(clan = clan_lookup(argument))) {
		stc("That is not a clan.\n", ch);
		return;
	}

	if (db_queryf("do_clanlist",
	    "SELECT name, title, rank, level, remort, cgroup FROM pc_index WHERE clan LIKE '%s'", db_esc(clan->name)) != SQL_OK)
		return;

	while (db_next_row() == SQL_OK) {
		Flags cgroup = db_get_column_flags(5);
		strcpy(clan_list[count].name, db_get_column_str(0));
		strcpy(clan_list[count].title, db_get_column_str(1));
		strcpy(clan_list[count].rank, db_get_column_str(2));
		clan_list[count].level  = db_get_column_int(3);
		clan_list[count].remort = db_get_column_int(4);
		clan_list[count].status = cgroup.has(GROUP_LEADER) ? 2 : cgroup.has(GROUP_DEPUTY) ? 1 : 0;
		clan_list[count].printed       = false;

		if (clan_list[count].remort > highremort)
			highremort = clan_list[count].remort;

		if (clan_list[count].level > highlevel)
			highlevel = clan_list[count].level;

		count++;
	}

	buffer += Format::format("{VMembers of %s:\n\n", clan->clanname);
	Format::sprintf(rblock, "{x");
	Format::sprintf(lblock, "{x");

	if (clan->independent)  /* we sort the imm clan by leader-deputy-imm ranks, not level */
		for (r = 2; r > -1; r--) {
			for (i = 0; i < count; i++) {
				if (clan_list[i].printed || clan_list[i].status != r)
					continue;

				buffer += Format::format("%s[{G%3s%s] {W[ {CIMM {W] %s%s{x\n",
				    r == 2 ? "{Y" : r == 1 ? "{B" : "{W",
				    clan_list[i].rank,
				    r == 2 ? "{Y" : r == 1 ? "{B" : "{W",
				    clan_list[i].name,
				    clan_list[i].title);
				clan_list[i].printed = true;
			}
		}
	else
		for (r = highremort; r > -1; r--) {
			for (l = highlevel; l > 0; l--) {
				for (i = 0; i < count; i++) {
					if (clan_list[i].printed == true
					    || clan_list[i].level != l
					    || (clan_list[i].remort != r
					        && clan_list[i].level < LEVEL_IMMORTAL))
						continue;

					if (l < LEVEL_IMMORTAL) {
						if (r)
							Format::sprintf(rblock, "{G%2d{c/", r);
						else
							Format::sprintf(rblock, "   ");

						Format::sprintf(lblock, "{W[%s{B%2d{W]", rblock, l);
					}
					else
						Format::sprintf(lblock, "{W[ {CIMM {W]");

					buffer += Format::format("%s[{G%3s%s] %s %s%s{x\n",
					    clan_list[i].status == 2 ? "{Y" :
					    clan_list[i].status == 1 ? "{B" : "{W",
					    clan_list[i].rank,
					    clan_list[i].status == 2 ? "{Y" :
					    clan_list[i].status == 1 ? "{B" : "{W",
					    lblock,
					    clan_list[i].name,
					    clan_list[i].title);
					clan_list[i].printed = true;
				}
			}
		}

	if (count == 0)
		stc("- none -\n", ch);
	else {
		buffer += Format::format("\n{WThere are %d members of %s{x.\n", count, clan->clanname);
		page_to_char(buffer, ch);
	}

} /* end do_clanlist() */

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(Character *ch, String argument)
{
	/* lists most player flags */
	if (ch->is_npc())
		return;

	stc("   action     status\n", ch);
	stc("---------------------\n", ch);
	stc("autoassist     ", ch);

	if (ch->act_flags.has(PLR_AUTOASSIST))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autoexit       ", ch);

	if (ch->act_flags.has(PLR_AUTOEXIT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autogold       ", ch);

	if (ch->act_flags.has(PLR_AUTOGOLD))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autoloot       ", ch);

	if (ch->act_flags.has(PLR_AUTOLOOT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autosac        ", ch);

	if (ch->act_flags.has(PLR_AUTOSAC))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autosplit      ", ch);

	if (ch->act_flags.has(PLR_AUTOSPLIT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autotick       ", ch);

	if (ch->act_flags.has(PLR_TICKS))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autorecall     ", ch);

	if (ch->act_flags.has(PLR_WIMPY))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("chatmode       ", ch);

	if (ch->pcdata->plr_flags.has(PLR_CHATMODE))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("compact mode   ", ch);

	if (ch->comm_flags.has(COMM_COMPACT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("prompt         ", ch);

	if (ch->comm_flags.has(COMM_PROMPT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("show defensive ", ch);

	if (ch->act_flags.has(PLR_DEFENSIVE))
		stc("OFF\n", ch);
	else
		stc("ON\n", ch);

	stc("show RAffects  ", ch);

	if (ch->pcdata->plr_flags.has(PLR_SHOWRAFF))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("combine items  ", ch);

	if (ch->comm_flags.has(COMM_COMBINE))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autopeek       ", ch);

	if (ch->pcdata->plr_flags.has(PLR_AUTOPEEK))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	if (ch->pcdata) {
		stc("showlost       ", ch);

		if (ch->pcdata->plr_flags.has(PLR_SHOWLOST))
			stc("ON\n", ch);
		else
			stc("OFF\n", ch);

		stc("flash mode     ", ch);

		if (ch->pcdata->video_flags.has(VIDEO_FLASH_LINE))
			stc("UNDERLINE\n", ch);
		else if (ch->pcdata->video_flags.has(VIDEO_FLASH_OFF))
			stc("OFF\n", ch);
		else
			stc("ON\n", ch);
	}

	if (!ch->act_flags.has(PLR_CANLOOT))
		stc("Your corpse is safe from thieves.\n", ch);
	else
		stc("Your corpse may be looted.\n", ch);

	if (ch->act_flags.has(PLR_NOSUMMON))
		stc("You cannot be summoned.\n", ch);
	else
		stc("You can be summoned.\n", ch);

	if (ch->act_flags.has(PLR_NOFOLLOW))
		stc("You do not welcome followers.\n", ch);
	else
		stc("You accept followers.\n", ch);

	if (ch->pcdata) {
		if (ch->pcdata->plr_flags.has(PLR_SHOWLOST))
			stc("You are notified about lost items when quiting.\n", ch);
		else
			stc("You are not notified about lost items when quiting.\n", ch);
	}
}

/* Color testing by Lotus */
void do_ctest(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_COLOR)) {
		stc("Colors currently used at Legacy:\n", ch);
		stc("\n", ch);
		stc("{WWHITE     (W)  {gGREY      (g)  {e{WBG GREY      (e){a\n", ch);
		stc("{PPINK      (P)  {RRED       (R)  {r{PBG RED       (r){a\n", ch);
		stc("{YYELLOW    (Y)  {bBROWN     (b)  {y{YBG BROWN     (y){a\n", ch);
		stc("{GGREEN     (G)  {HHUNTER    (H)  {h{GBG HUNTER    (h){a\n", ch);
		stc("{CCYAN      (C)  {TTURQUOISE (T)  {t{CBG TURQUOISE (t){a\n", ch);
		stc("{BBLUE      (B)  {NNAVY      (N)  {n{BBG NAVY      (n){a\n", ch);
		stc("{VVIOLET    (V)  {MMAROON    (M)  {m{VBG MAROON    (m){a\n", ch);
		stc("{cCHARCOAL  (c)  {kBLACK     (k)  {a{cBG BLACK     (a){a\n", ch);
		stc("{PD{YE{GF{BA{MU{VL{WT   {x(x)  {fFLASHING  (f){x  {sREVERSE      (s){x\n", ch);
	}
	else
		stc("You must turn color on to see this.\n", ch);
}

void do_autoassist(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOASSIST)) {
		stc("Autoassist removed.\n", ch);
		ch->act_flags -= PLR_AUTOASSIST;
	}
	else {
		stc("You will now assist when needed.\n", ch);
		ch->act_flags += PLR_AUTOASSIST;
	}
}

void do_defensive(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_DEFENSIVE)) {
		stc("You will now see all blocks.\n", ch);
		ch->act_flags -= PLR_DEFENSIVE;
	}
	else {
		stc("You will no longer see defensive blocks.\n", ch);
		ch->act_flags += PLR_DEFENSIVE;
	}
}

void do_autoexit(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOEXIT)) {
		stc("Exits will no longer be displayed.\n", ch);
		ch->act_flags -= PLR_AUTOEXIT;
	}
	else {
		stc("Exits will now be displayed.\n", ch);
		ch->act_flags += PLR_AUTOEXIT;
	}
}

void do_autogold(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOGOLD)) {
		stc("Autogold removed.\n", ch);
		ch->act_flags -= PLR_AUTOGOLD;
	}
	else {
		stc("Automatic gold looting set.\n", ch);
		ch->act_flags += PLR_AUTOGOLD;
	}
}

void do_autoloot(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOLOOT)) {
		stc("Autolooting removed.\n", ch);
		ch->act_flags -= PLR_AUTOLOOT;
	}
	else {
		stc("Automatic corpse looting set.\n", ch);
		ch->act_flags += PLR_AUTOLOOT;
	}
}

void do_autosac(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOSAC)) {
		stc("Autosacrificing removed.\n", ch);
		ch->act_flags -= PLR_AUTOSAC;
	}
	else {
		stc("Automatic corpse sacrificing set.\n", ch);
		ch->act_flags += PLR_AUTOSAC;
	}
}

void do_autosplit(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_AUTOSPLIT)) {
		stc("Autosplitting removed.\n", ch);
		ch->act_flags -= PLR_AUTOSPLIT;
	}
	else {
		stc("Automatic gold splitting set.\n", ch);
		ch->act_flags += PLR_AUTOSPLIT;
	}
}

void do_brief(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_BRIEF)) {
		stc("Full descriptions activated.\n", ch);
		ch->comm_flags -= COMM_BRIEF;
	}
	else {
		stc("Short descriptions activated.\n", ch);
		ch->comm_flags += COMM_BRIEF;
	}
}

void do_compact(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_COMPACT)) {
		stc("Compact mode removed.\n", ch);
		ch->comm_flags -= COMM_COMPACT;
	}
	else {
		stc("Compact mode set.\n", ch);
		ch->comm_flags += COMM_COMPACT;
	}
}

void do_showflags(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	victim = ch;

	if (IS_IMMORTAL(ch) && !argument.empty()) {
		if ((victim = get_char_world(ch, argument, VIS_PLR)) == nullptr) {
			stc("You cannot locate that character.\n", ch);
			return;
		}
	}

	set_color(ch, YELLOW, NOBOLD);
	Format::sprintf(buf, "Act  : %s\n", act_bit_name(victim->act_flags, victim->is_npc()));
	stc(buf, ch);

	if (!victim->is_npc()) {
		Format::sprintf(buf, "Plr  : %s\n", plr_bit_name(victim->pcdata->plr_flags));
		stc(buf, ch);
	}

	if (victim->is_npc()) {
		Format::sprintf(buf, "Off  : %s\n", off_bit_name(victim->off_flags));
		stc(buf, ch);
	}

	if (!victim->is_npc()) {
		Format::sprintf(buf, "Wiz  : %s\n", wiz_bit_name(victim->wiznet_flags));
		stc(buf, ch);
	}

	ptc(ch, "Aff  : %s\n", affect::print_cache(victim));
	ptc(ch, "Drn  : %s\n", print_defense_modifiers(victim, TO_ABSORB));
	ptc(ch, "Imm  : %s\n", print_defense_modifiers(victim, TO_IMMUNE));
	ptc(ch, "Res  : %s\n", print_defense_modifiers(victim, TO_RESIST));
	ptc(ch, "Vuln : %s\n", print_defense_modifiers(victim, TO_VULN));
	ptc(ch, "Form : %s\n", form_bit_name(victim->form_flags));
	ptc(ch, "Parts: %s\n", part_bit_name(victim->parts_flags));
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_show(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_SHOW_AFFECTS)) {
		stc("Affects will no longer be shown in score.\n", ch);
		ch->comm_flags -= COMM_SHOW_AFFECTS;
	}
	else {
		stc("Affects will now be shown in score.\n", ch);
		ch->comm_flags += COMM_SHOW_AFFECTS;
	}
}

#define MAX_PROMPT_LEN 300
void do_prompt(Character *ch, String argument)
{
	String buf;

	if (argument.empty()) {
		if (ch->comm_flags.has(COMM_PROMPT)) {
			stc("You will no longer see prompts.\n", ch);
			ch->comm_flags -= COMM_PROMPT;
		}
		else {
			stc("You will now see prompts.\n", ch);
			ch->comm_flags += COMM_PROMPT;
		}

		return;
	}

	if (!strcmp(argument, "default"))
		buf = "{W<{C%h{Thp {G%m{Hma {B%v{Nst {W[{g%e{W] {R%X{W>{x ";
	else if (!strcmp(argument, "short"))
		buf = "{W<{C%h{Thp {G%m{Hma {B%v{Nst{W>{x ";
	else if (!strcmp(argument, "long"))
		buf = "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{c%e{W] {Y%g{C/{g%s {W( {g%z {W) {R%X{W>{x ";
	else if (!strcmp(argument, "riches"))
		buf = "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{g%e{W] {Y%g{bg {W%s{gs{W>{x ";
	else if (!strcmp(argument, "immortal"))
		buf = "{W<{Y({W%R{Y) {T-{C%r{T- {H[{G%e{H] {W%z {N[{B%w{N]{W>{x ";
	else if (!strcmp(argument, "align"))
		buf = "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{c%e{W] {R%X {V({M%a{V){W>{x ";
	else if (!strcmp(argument, "quest"))
		buf = "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {Y({b%Q %q{Y){W>{x ";
	else {
		buf += argument;
		buf.erase(MAX_PROMPT_LEN);

		if (buf.has_suffix("%c"))
			buf += " ";
	}

	ch->prompt = buf;
	Format::sprintf(buf, "Prompt set to %s\n", ch->prompt);
	stc(buf, ch);
}

void do_combine(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_COMBINE)) {
		stc("Long inventory selected.\n", ch);
		ch->comm_flags -= COMM_COMBINE;
	}
	else {
		stc("Combined inventory selected.\n", ch);
		ch->comm_flags += COMM_COMBINE;
	}
}

void do_showlost(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->pcdata->plr_flags.has(PLR_SHOWLOST)) {
		stc("You will no longer be notified of lost items.\n", ch);
		ch->pcdata->plr_flags -= PLR_SHOWLOST;
	}
	else {
		stc("You will now be notified of lost items.\n", ch);
		ch->pcdata->plr_flags += PLR_SHOWLOST;
	}
}

void do_noloot(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_CANLOOT)) {
		stc("Your corpse is now safe from thieves.\n", ch);
		ch->act_flags -= PLR_CANLOOT;
	}
	else {
		stc("Your corpse may now be looted.\n", ch);
		ch->act_flags += PLR_CANLOOT;
	}
}

void do_nofollow(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->act_flags.has(PLR_NOFOLLOW)) {
		stc("You now accept followers.\n", ch);
		ch->act_flags -= PLR_NOFOLLOW;
	}
	else {
		stc("You no longer accept followers.\n", ch);
		ch->act_flags += PLR_NOFOLLOW;
		die_follower(ch);
	}
}

void do_nosummon(Character *ch, String argument)
{
	if (ch->act_flags.has(PLR_NOSUMMON)) {
		stc("You are no longer immune to summon.\n", ch);
		ch->act_flags -= PLR_NOSUMMON;
	}
	else {
		stc("You are now immune to summoning.\n", ch);
		ch->act_flags += PLR_NOSUMMON;
	}
}

void do_look(Character *ch, String argument)
{
	char buf  [MAX_STRING_LENGTH];
	Exit *pexit;
	Character *victim;
	Object *obj;
	const ExtraDescr *pdesc;
	int door;
	int number, count;

	if (ch->desc == nullptr)
		return;

	if (get_position(ch) < POS_SLEEPING) {
		stc("You can't see anything!\n", ch);
		return;
	}

	if (get_position(ch) == POS_SLEEPING) {
		stc("You can't see anything, you're sleeping!\n", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!can_see_in_room(ch, ch->in_room)) {
		stc("It is pitch black ... \n", ch);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	number = number_argument(arg1, arg3);
	count = 0;

	if (arg1.empty() || arg1 == "auto") {
		if (ch->in_room == nullptr)
			return;

		std::vector<String> minimap;
		Game::world().get_minimap(ch, minimap); // fill it, could be empty
		unsigned int map_line = 0;

		if (minimap.size() > map_line)
			stc(minimap[map_line++] + "  ", ch);

		/* 'look' or 'look auto' */
		new_color(ch, CSLOT_MISC_ROOM);
		stc(ch->in_room->name(), ch);
		set_color(ch, WHITE, NOBOLD);

		if (IS_IMMORTAL(ch)) {
			Format::sprintf(buf, " [%s] (%s) (%d)",
			        ch->in_room->location.to_string(),
			        ch->in_room->area().name,
			        ch->in_room->area().num_players());
			set_color(ch, WHITE, BOLD);
			stc(buf, ch);
			set_color(ch, WHITE, NOBOLD);
		}

		stc("\n", ch);

		String text;

		if (arg1.empty()
		    || (!ch->is_npc() && !ch->comm_flags.has(COMM_BRIEF))) {
			text = "  " + ch->in_room->description();
		}

		if (!ch->is_npc() && ch->act_flags.has(PLR_AUTOEXIT)) {
			text += "\n" + list_exits(ch, true);
		}

		std::istringstream textstream(text);
		String line;

		while (std::getline(textstream, line)) {
			if (minimap.size() > map_line)
				stc(minimap[map_line++] + "  ", ch);

			set_color(ch, WHITE, NOBOLD);
			stc(line + "\n", ch);
		}

		while (minimap.size() > map_line)
			stc(minimap[map_line++] + "\n", ch);

		new_color(ch, CSLOT_MISC_OBJECTS);
		show_list_to_char(ch->in_room->contents, ch, false, false, false);
		set_color(ch, WHITE, NOBOLD);
		show_char_to_char(ch->in_room->people,   ch);
		return;
	}

	/* viewing room stuff */
	{
		Duel *duel = duel_table_head->next;

		while (duel != duel_table_tail) {
			if (duel->accept_timer == 0 && ch->in_room == duel->arena->viewroom)
				break;

			duel = duel->next;
		}

		if (duel != duel_table_tail
		    && duel->challenger != nullptr
		    && duel->defender != nullptr
		    && ch != duel->challenger
		    && ch != duel->defender) {
			if (can_see_char(ch, duel->challenger)
			    && !duel->challenger->in_room->flags().has(ROOM_NOWHERE)
			    && arg1.is_prefix_of(duel->challenger->name)) {
				char_from_room(ch);
				char_to_room(ch, duel->challenger->in_room);
				do_look(ch, "");
				char_from_room(ch);
				char_to_room(ch, duel->arena->viewroom);
				return;
			}

			if (can_see_char(ch, duel->defender)
			    && !duel->defender->in_room->flags().has(ROOM_NOWHERE)
			    && arg1.is_prefix_of(duel->defender->name)) {
				char_from_room(ch);
				char_to_room(ch, duel->defender->in_room);
				do_look(ch, "");
				char_from_room(ch);
				char_to_room(ch, duel->arena->viewroom);
				return;
			}
		}
	}

	if (arg1 == "i" || arg1 == "in" || arg1 == "inside" || arg1 == "on") {
		Room *location;

		/* 'look in' */
		if (arg2.empty()) {
			stc("Look in what?\n", ch);
			return;
		}

		/* Stuff for Lockers */
		if (arg2.is_prefix_of("locker") && !ch->is_npc()) {
			if (ch->in_room->flags().has(ROOM_LOCKER)) {
				stc("Your locker contains:\n", ch);
				ch->act_flags += PLR_LOOKINPIT;
				show_list_to_char(ch->pcdata->locker, ch, true, true, false);
				ch->act_flags -= PLR_LOOKINPIT;
			}
			else
				stc("You do not see that here.\n", ch);

			return;
		}

		/* Stuff for strongboxes -- Elrac */
		if (!ch->is_npc() && arg2.is_prefix_of("strongbox")) {
			if (!IS_HEROIC(ch)) {
				stc("Only heroes and former heroes have a strongbox.\n", ch);
				return;
			}

			if (ch->in_room && ch->in_room->location == Location(Vnum(ROOM_VNUM_STRONGBOX))) {
				stc("Your strongbox contains:\n", ch);
				ch->act_flags += PLR_LOOKINPIT;
				show_list_to_char(ch->pcdata->strongbox, ch, true, true, false);
				ch->act_flags -= PLR_LOOKINPIT;
			}
			else
				stc("You do not see that here.\n", ch);

			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == nullptr) {
			stc("You do not see that here.\n", ch);
			return;
		}

		/*
		Some of the reasoning for checking contents AND whether the item is ITEM_COMPARTMENT
		may seem irrational, but it's actually so you can see the contents of an object that
		you can't get objects from.  Some uses might include glass cases in shops, a scroll
		sealed inside a crystal shard, etc.  These would have an object reset inside them,
		or be flagged ITEM_COMPARTMENT long enough to receive an item, and then the object
		inside could be completely ungettable, or gotten out through destroying the item.
		                                                        -- Montrey
		*/
		switch (obj->item_type) {
		default:
			if (!IS_OBJ_STAT(obj, ITEM_COMPARTMENT) && !obj->contains)
				stc("That is not a container.\n", ch);

			break;

		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0 && !obj->contains) {
				stc("It is empty.\n", ch);
				break;
			}

			ptc(ch, "It's %sfilled with  a %s liquid.\n",
			    obj->value[1] < obj->value[0] / 4 ?     "less than half-" :
			    obj->value[1] < 3 * obj->value[0] / 4 ? "about half-" :
			    "more than half-",
			    liq_table[obj->value[2]].color);
			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (obj->value[1].flags().has(CONT_CLOSED)) {
				stc("It is closed.\n", ch);
				break;
			}

			if (obj == Game::world().donation_pit)
				ch->act_flags += PLR_LOOKINPIT;

			new_color(ch, CSLOT_MISC_OBJECTS);
			act("$p holds:", ch, obj, nullptr, TO_CHAR);
			show_list_to_char(obj->contains, ch, true, true, false);
			set_color(ch, WHITE, NOBOLD);

			if (obj == Game::world().donation_pit)
				ch->act_flags -= PLR_LOOKINPIT;

			break;

		case ITEM_PORTAL:
			if ((location = Game::world().get_room(Location((int)obj->value[3]))) == nullptr) {
				if (!obj->contains)
					stc("It looks very empty..\n", ch);

				break;
			}

			stc("\n", ch);
			new_color(ch, CSLOT_MISC_ROOM);
			stc(location->name(), ch);
			set_color(ch, WHITE, NOBOLD);

			if (IS_IMMORTAL(ch)) {
				Format::sprintf(buf, " [Room %s]", location->location.to_string());
				set_color(ch, RED, NOBOLD);
				stc(buf, ch);
				set_color(ch, WHITE, NOBOLD);
			}

			stc("\n", ch);

			if (!ch->is_npc() && !ch->comm_flags.has(COMM_BRIEF)) {
				stc("  ", ch);
				stc(location->description(), ch);
			}

			stc("\n", ch);

			if (!ch->is_npc() && ch->act_flags.has(PLR_AUTOEXIT)) {
				char showexit[100];
				Format::sprintf(showexit, "%s exits auto", location->location.to_string());
				do_at(ch, showexit);
			}

			new_color(ch, CSLOT_MISC_OBJECTS);
			show_list_to_char(location->contents, ch, false, false, false);
			set_color(ch, WHITE, NOBOLD);
			show_char_to_char(location->people, ch);
			break;
		}

		if (obj->item_type != ITEM_CONTAINER    /* these 3 are containers regardless */
		    && obj->item_type != ITEM_CORPSE_NPC
		    && obj->item_type != ITEM_CORPSE_PC
		    && (IS_OBJ_STAT(obj, ITEM_COMPARTMENT) || obj->contains)) {
			if (!IS_OBJ_STAT(obj, ITEM_COMPARTMENT) && !IS_OBJ_STAT(obj, ITEM_TRANSPARENT))
				stc("The surface is opaque.\n", ch);
			else {
				new_color(ch, CSLOT_MISC_OBJECTS);
				act("$p holds:", ch, obj, nullptr, TO_CHAR);
				show_list_to_char(obj->contains, ch, true, true, false);
				set_color(ch, WHITE, NOBOLD);
			}
		}

		return;
	}

	if ((victim = get_char_here(ch, arg1, VIS_CHAR)) != nullptr) {
		show_char_to_char_1(victim, ch);
		return;
	}

	for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			/* player can see object */
			pdesc = get_extra_descr(arg3, obj->extra_descr);

			if (pdesc != nullptr) {
				if (++count == number) {
					stc(pdesc->description, ch);
					return;
				}
				else continue;
			}

			pdesc = get_extra_descr(arg3, obj->pIndexData->extra_descr);

			if (pdesc != nullptr) {
				if (++count == number) {
					stc(pdesc->description, ch);
					return;
				}
				else continue;
			}

			if (obj->name.has_words(arg3)) {
				if (++count == number) {
					stc(obj->description, ch);
					stc("\n", ch);
					return;
				}
			}
		}
	}

	for (obj = ch->in_room->contents; obj != nullptr; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			pdesc = get_extra_descr(arg3, obj->extra_descr);

			if (pdesc != nullptr) {
				if (++count == number) {
					stc(pdesc->description, ch);
					return;
				}
				else
					continue;
			}

			pdesc = get_extra_descr(arg3, obj->pIndexData->extra_descr);

			if (pdesc != nullptr) {
				if (++count == number) {
					stc(pdesc->description, ch);
					return;
				}
				else
					continue;
			}
		}

		if (obj->name.has_words(arg3))
			if (++count == number) {
				stc(obj->description, ch);
				stc("\n", ch);
				return;
			}
	}

	pdesc = get_extra_descr(arg3, ch->in_room->extra_descr());

	if (pdesc != nullptr) {
		if (++count == number) {
			stc(pdesc->description, ch);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			Format::sprintf(buf, "You only see one %s here.\n", arg3);
		else
			Format::sprintf(buf, "You only see %d of those here.\n", count);

		stc(buf, ch);
		return;
	}

	if (arg1 == "n" || arg1 == "north") door = 0;
	else if (arg1 == "e" || arg1 == "east") door = 1;
	else if (arg1 == "s" || arg1 == "south") door = 2;
	else if (arg1 == "w" || arg1 == "west") door = 3;
	else if (arg1 == "u" || arg1 == "up") door = 4;
	else if (arg1 == "d" || arg1 == "down") door = 5;
	else {
		stc("You do not see that here.\n", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == nullptr) {
		stc("Nothing special there.\n", ch);
		return;
	}

	if (!pexit->description().empty())
		stc(pexit->description(), ch);
	else
		stc("Nothing special there.\n", ch);

	if (!pexit->keyword().empty()
	    &&   pexit->keyword()[0] != ' ') {
		if (pexit->exit_flags.has(EX_CLOSED))
			act("The $d is closed.", ch, nullptr, pexit->keyword(), TO_CHAR);
		else if (pexit->exit_flags.has(EX_ISDOOR))
			act("The $d is open.",   ch, nullptr, pexit->keyword(), TO_CHAR);
	}

	return;
}

void do_examine(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *obj;

	if (argument.empty()) {
		stc("Examine what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	/* enable "examine locker" -- Elrac */
	if (argument.is_prefix_of("locker"))
		arg = "in locker";

	if (argument.is_prefix_of("strongbox"))
		arg = "in strongbox";

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != nullptr) {
		switch (obj->item_type) {
		default:
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT) || obj->contains) {
				Format::sprintf(buf, "in %s", argument);
				do_look(ch, buf);
			}

			break;

		case ITEM_JUKEBOX:
			do_play(ch, "list");
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0) {
				if (obj->value[1] == 0)
					Format::sprintf(buf, "Odd...there's no coins in the pile.\n");
				else if (obj->value[1] == 1)
					Format::sprintf(buf, "Wow. One gold coin.\n");
				else
					Format::sprintf(buf, "There are %d gold coins in the pile.\n", obj->value[1]);
			}
			else if (obj->value[1] == 0) {
				if (obj->value[0] == 1)
					Format::sprintf(buf, "Wow. One silver coin.\n");
				else
					Format::sprintf(buf, "There are %d silver coins in the pile.\n", obj->value[0]);
			}
			else
				Format::sprintf(buf, "There are %d gold and %d silver coins in the pile.\n",
				        obj->value[1], obj->value[0]);

			stc(buf, ch);
			break;

		case ITEM_PORTAL:
		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			Format::sprintf(buf, "in %s", argument);
			do_look(ch, buf);
			break;
		}
	}
}

/*
** Show the rooms && portals which have exits leading INTO the current room
** -DF ( 04-06-98 )
*/
void exits_in(Character *ch)
{
	Exit *exit;
	Object *obj;
	String output;
	char buf[1024];
	bool found = false;

	if (ch->in_room == nullptr)
		return;

	for (const auto& area_pair : Game::world().areas) {
		for (const auto& pair : area_pair.second->rooms) {
			const Room *room = pair.second;

			for (int i = 0; i < 6; i++) { /* Every exit in the current room */
				exit = room->exit[i];

				if (exit == nullptr)
					continue;

				if (exit->to_room == nullptr)
					continue;

				if (exit->to_room == ch->in_room) {   /* Does the exit lead to our room? */
					found = true;
					Format::sprintf(buf, "( %-6.6s ) from %s (%s) in (%s)\n",
					        Exit::dir_name(i),
					        room->name(),
					        room->location.to_string(),
					        room->area().name);
					output += buf;
				} /* End if */
			} /* End for */
		} /* End for */
	}

	if (!found)
		output += "No rooms lead into this one.\n";

	found = false;

	for (obj = Game::world().object_list; obj != nullptr; obj = obj->next) {
		if (obj->item_type == ITEM_PORTAL) {
			if (obj->in_room == nullptr)
				continue;

			if (Location((int)obj->value[3].value()) == ch->in_room->location) {
				found = true;
				Format::sprintf(buf, "( Portal ) %s in %s (%s) in (%s)\n",
				        obj->name,
				        obj->in_room->name(),
				        obj->in_room->location.to_string(),
				        obj->in_room->area().name);
				output += buf;
			}                                                          /* End if */
		}
	}

	if (!found)
		output += "No portals found that lead into this room.\n";

	page_to_char(output, ch);
} /* End do_exits_in */

const String list_exits(Character *ch, bool fAuto) {
	String buf;

	if (fAuto)
		Format::sprintf(buf, "[Exits:");
	else if (IS_IMMORTAL(ch))
		Format::sprintf(buf, "Obvious exits from room %s:\n", ch->in_room->location.to_string());
	else
		Format::sprintf(buf, "Obvious exits:\n");

	bool found = false;

	for (int door = 0; door <= 5; door++) {
		const Exit *pexit;

		if ((pexit = ch->in_room->exit[door]) != nullptr
		    &&   pexit->to_room != nullptr
		    &&   can_see_room(ch, pexit->to_room)) {
//		    &&   !pexit->exit_flags.has(EX_CLOSED)) {

			found = true;

			if (fAuto) {
				buf += " ";
				if (pexit->exit_flags.has(EX_CLOSED))
					buf += Format::format("{Y(%s){x", Exit::dir_name(door));
				else
					buf += Exit::dir_name(door);
			}
			else {
				buf += Format::format("%-5s - ", Exit::dir_name(door).capitalize());

				if (pexit->exit_flags.has(EX_CLOSED))
					buf += "{Y(closed){x";
				else if ((pexit->to_room->is_dark() && !affect::exists_on_char(ch, affect::type::night_vision))
					  || pexit->to_room->is_very_dark())
					buf += "{cToo dark to tell{x";
				else
					buf += pexit->to_room->name();

				if (IS_IMMORTAL(ch))
					buf += Format::format(" (room %s)\n", pexit->to_room->location.to_string());
				else
					buf += "\n";
			}
		}
	}

	if (!found)
		buf += fAuto ? " none" : "None.\n";

	if (fAuto)
		buf += "]\n";

	return buf;
}
/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(Character *ch, String argument)
{
	if (!check_blind(ch))
		return;

	if (argument == "in" && IS_IMMORTAL(ch)) {
		exits_in(ch);
		return;
	}

	bool fAuto = argument == "auto";
	String buf = list_exits(ch, fAuto);

	set_color(ch, WHITE, BOLD);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_worth(Character *ch, String argument)
{
	if (ch->is_npc()) {
		ptc(ch, "You have %ld gold and %ld silver.\n", ch->gold, ch->silver);
		return;
	}

	if (ch->level < LEVEL_HERO)
		ptc(ch, "You have %ld gold, %ld silver, and %d experience (%ld exp to level).\n",
		    ch->gold, ch->silver, ch->exp, (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp);
	else
		ptc(ch, "You have %ld gold, %ld silver, and %d experience.\n", ch->gold, ch->silver, ch->exp);

	ptc(ch, "You have %ld gold and %ld silver in the bank.\n", ch->gold_in_bank, ch->silver_in_bank);
	ptc(ch, "You have earned %d quest points, %d skill points, %d roleplay points,\n",
	    ch->pcdata->questpoints, ch->pcdata->skillpoints, ch->pcdata->rolepoints);
	ptc(ch, "%d trains, and %d practices.\n", ch->train, ch->practice);
	return;
}

void do_time(Character *ch, String argument)
{
	const World& world = ch->in_room->area().world;

	ptc(ch, "{WIt is %d o'clock %s, Day of %s, %s of the Month of %s.{x\n",
		(world.time.hour % 12 == 0) ? 12 : world.time.hour % 12,
		world.time.hour >= 12 ? "pm" : "am",
		world.time.day_name(),
		world.time.day_string(),
		world.time.month_name());

	if (Game::quest_double) {
		stc("{gQuest points are currently being doubled!", ch);

		if (IS_IMMORTAL(ch)) {
			int hours       = Game::quest_double / 3600,
			    minutes     = (Game::quest_double - (hours * 3600)) / 60,
			    seconds     = (Game::quest_double - (hours * 3600)) - (minutes * 60);

			if (Game::quest_double > 1200)
				stc("{Y", ch);

			stc(" (", ch);
			ptc(ch, "%2d:", hours);

			if (minutes > 9)
				ptc(ch, "%d:", minutes);
			else
				ptc(ch, "0%d:", minutes);

			if (seconds > 9)
				ptc(ch, "%d)", seconds);
			else
				ptc(ch, "0%d)", seconds);
		}

		stc("{x\n", ch);
	}

	extern char str_boot_time[];

	ptc(ch, "{gLegacy started up %s\nThe system time is: %s{x\n",
		str_boot_time, (char *) ctime(&Game::current_time));
}

void do_weather(Character *ch, String argument)
{
	if (!IS_OUTSIDE(ch)) {
		stc("You are inside and cannot see outdoors.\n", ch);
		return;
	}

	ptc(ch, "%s\n", ch->in_room->area().world.weather.describe());
}

/* new whois by Montrey */
void do_whois(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	String block;
	char clan[MAX_STRING_LENGTH];
	String rank;
	char *remort;
	String output;
	Character *victim;

	if (argument.empty()) {
		stc("You must provide a name.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
		stc("No one of that name is playing.\n", ch);
		return;
	}

	/* first block "(RP)(PB)(NH) " */
	Format::sprintf(block, "%s%s%s",
	        victim->pcdata->plr_flags.has(PLR_OOC) ? "{W({YRP{W){x" : "",
	        victim->pcdata->plr_flags.has(PLR_PAINT) ? "{W({VPB{W){x" : "",
	        victim->act_flags.has(PLR_MAKEBAG) ? "{W({CNH{W){x" : "");
	output += block;

	if (!block.empty())
		output += " ";

	/* second block "[99 100 Sup Pdn]"*/
	if (IS_REMORT(victim)) {
		Format::sprintf(buf, "{G%2d{x", victim->pcdata->remort_count);
		remort = buf;
	}
	else
		remort = "  ";

	Format::sprintf(block, "%s%s {B%d {P%3s {C%3s%s ",
	        (victim->pcdata->plr_flags.has(PLR_PK)) ? "{P[{x" : "{g[{x",
	        remort,
	        victim->level,
	        pc_race_table[victim->race].who_name,
	        victim->guild == Guild::none ? "---" : guild_table[victim->guild].who_name,
	        (victim->pcdata->plr_flags.has(PLR_PK)) ? "{P]{x" : "{g]{x");
	output += block;
	/* third block "name title" */
	Format::sprintf(block, "%s%s%s{W%s{x %s{x\n",
	        (victim->comm_flags.has(COMM_AFK)) ? "{b[AFK]{x " : "",
	        (victim->act_flags.has(PLR_KILLER)) ? "{R(KILLER){x " : "",
	        (victim->act_flags.has(PLR_THIEF)) ? "{B(THIEF){x " : "",
	        victim->name,
	        victim->pcdata->title);
	output += block;
	/* second line */
	Format::sprintf(clan, "{x"); /* ugly, do something better someday */

	if (victim->clan != nullptr) {
		if (!victim->pcdata->rank.empty())
			rank = victim->pcdata->rank;
		else if (victim->has_cgroup(GROUP_LEADER))
			rank = "Leader";
		else if (victim->has_cgroup(GROUP_DEPUTY))
			rank = "Deputy";
		else
			rank = "Member";

		Format::sprintf(clan, "%s{x of %s{x, ", rank, victim->clan->clanname);
	}

	if (IS_REMORT(victim) && !victim->pcdata->status.empty())
		Format::sprintf(remort, "%s{x, ", victim->pcdata->status);
	else
		remort = "";

	Format::sprintf(block, "%s%s{PRating: %d{x\n", clan, remort, victim->pcdata->pkrank);
	output += block;

	if (victim->comm_flags.has(COMM_AFK))
		output += victim->pcdata->afk;

	page_to_char(output, ch);
}

char *count_players(Character *ch)
{
	static char buf[100];
	int count = 0;
	Descriptor *d;

	for (d = descriptor_list; d != nullptr; d = d->next)
		if (d->is_playing() && can_see_who(ch, d->character))
			count++;

	if (Game::record_players_since_boot == count)
		Format::sprintf(buf, "There are %d characters on, the most since last reboot.\n", count);
	else
		Format::sprintf(buf, "There are %d characters on, the most since last reboot is %d.\n",
		        count, Game::record_players_since_boot);

	return buf;
}

void do_count(Character *ch, String argument)
{
	set_color(ch, WHITE, BOLD);
	stc(count_players(ch), ch);
	ptc(ch, "The most players ever on at once is %d.\n", Game::record_players);
	set_color(ch, WHITE, NOBOLD);
}

void do_who(Character *ch, String argument)
{
	struct s_charitem {
		Character *pch;
		long levelkey;
	}
	tmp_charitem;
	String buf;
	char buf2[MIL];
	String block1;  /* [level race class] */
	char block2[MIL];  /* [rank clan] */
	char rbuf[32];
	String output;
	Descriptor *d;
	int iLevelLower = 0, iLevelUpper = MAX_LEVEL;
	int nNumber = 0, nMatch = 0, ndesc = 0;
	int j1, j2;
	bool rgfClass[Guild::size], rgfRace[pc_race_table.size()];
	bool fClassRestrict = false, fClanRestrict = false, fRaceRestrict = false, fImmortalOnly = false;
	bool fClan = false;
	bool fPK = false;
	Character *wch;
	Clan *cch = nullptr;
	char *rank, *lbrk, *rbrk, *remort;

	/* Set default arguments. */
	for (unsigned long i = 0; i < Guild::size; i++)
		rgfClass[i] = false;

	for (unsigned long i = 0; i < pc_race_table.size(); i++)
		rgfRace[i] = false;

	/* Parse arguments. */
	for (; ;) {
		if (argument.empty())
			break;

		String arg;
		argument = one_argument(argument, arg);

		if (arg.is_number()) {
			switch (++nNumber) {
			case 1:         iLevelLower = atoi(arg);        break;

			case 2:         iLevelUpper = atoi(arg);        break;

			default:
				stc("Find players between what two levels?\n", ch);
				return;
			}
		}
		else {
			/* Look for classes to turn on. */
			if (arg[0] == 'i')
				fImmortalOnly = true;
			else if (arg.is_prefix_of("pk"))
				fPK = true;
			else if (arg == "clan")
				fClanRestrict = true;
			else {
				Guild guild = guild_lookup(arg);

				if (guild == Guild::none) {
					unsigned long iRace = race_lookup(arg);

					if (iRace == 0 || iRace >= pc_race_table.size()) {
						if ((cch = clan_lookup(arg)) != nullptr)
							fClan = true;
						else {
							stc("That's not a valid race, class, clan, or way to crash this mud.\n", ch);
							return;
						}
					}
					else {
						fRaceRestrict = true;
						rgfRace[iRace] = true;
					}
				}
				else {
					fClassRestrict = true;
					rgfClass[guild] = true;
				}
			}
		}
	}

	/* Count up descriptors to get max # of players */
	for (d = descriptor_list; d != nullptr; d = d->next)
		ndesc++;

	struct s_charitem charitems[ndesc];

	/* Now show matching chars. */
	buf.clear();

	for (d = descriptor_list; d != nullptr; d = d->next) {
		/* Check for match against restrictions. */
		if (!d->is_playing() || !can_see_who(ch, d->character))
			continue;

		wch = d->original ? d->original : d->character;

		if (!can_see_who(ch, wch))
			continue;

		if (wch->level < iLevelLower
		    || wch->level > iLevelUpper
		    || (fImmortalOnly && !IS_IMMORTAL(wch))
		    || (fPK && !wch->pcdata->plr_flags.has(PLR_PK))
		    || (fClassRestrict && !rgfClass[wch->guild])
		    || (fRaceRestrict && !rgfRace[wch->race])
		    || (fClan && wch->clan != cch)
		    || (fClanRestrict && !wch->clan))
			continue;

		charitems[nMatch].pch = wch;

		/* add in a fudge factor to force sort order for major groups */
		if (IS_IMMORTAL(wch)) {
			charitems[nMatch].levelkey = 99000;

			if (IS_IMP(wch))
				charitems[nMatch].levelkey += 2000;
			else if (IS_HEAD(wch))
				charitems[nMatch].levelkey += 1000;
		}
		else
			charitems[nMatch].levelkey = 1000 * wch->pcdata->remort_count + wch->level;

		nMatch++;
	}

	/* Sort the players on level. */
	for (j1 = 0; j1 < nMatch - 1; j1++) {
		for (j2 = j1 + 1; j2 < nMatch; j2++) {
			if (charitems[j2].levelkey > charitems[j1].levelkey) {
				tmp_charitem = charitems[j1];
				charitems[j1] = charitems[j2];
				charitems[j2] = tmp_charitem;
			}
		}
	}

	/* Print sorted player list. */
	for (j1 = 0; j1 < nMatch; j1++) {
		wch = charitems[j1].pch;
		/*** Block 1 stuff ***/
		/* Imm:    Immname */
		/* Mortal: [lvl race class] */
		String guild = wch->guild == Guild::none ? "---" : guild_table[wch->guild].who_name;

		if (IS_REMORT(wch)) {
			Format::sprintf(rbuf, "{G%2d{x", wch->pcdata->remort_count);
			remort = rbuf;
		}
		else
			remort = "  ";

		if (IS_IMMORTAL(wch) && !wch->pcdata->immname.empty()) {
			block1 = wch->pcdata->immname.center(15);
		}
		else {
			if (char_at_war(wch)) {
				lbrk = "<";
				rbrk = ">";
			}
			else {
				lbrk = "[";
				rbrk = "]";
			}

			Format::sprintf(block1, "%s%s%s {B%2d {P%3s {C%3s%s%s{x",
			        wch->pcdata->plr_flags.has(PLR_PK) ? "{P" : "{W",
			        lbrk,
			        remort,
			        wch->level,
			        pc_race_table[wch->race].who_name,
			        guild,
			        wch->pcdata->plr_flags.has(PLR_PK) ? "{P" : "{W",
			        rbrk);
		}

		/*** Block 2 stuff ***/
		/* Unclanned: "          "      */
		/* Leader:    [clanname *rank*]  */
		/* Other:     [clanname rank]  */

		if (wch->clan || IS_IMMORTAL(wch)) {
			Format::sprintf(rbuf, "%s{x%3s{x",
			        wch->has_cgroup(GROUP_LEADER) ? "{Y~" :
			        wch->has_cgroup(GROUP_DEPUTY) ? "{B~" : " ",
			        !wch->pcdata->rank.empty() ? wch->pcdata->rank :
			        IS_IMMORTAL(wch) ? "{WIMM" : "   ");
			rank = rbuf;
		}
		else
			rank = "    ";

		Format::sprintf(block2, "%s %s{a{x %s%s%s",
		        rank,
		        wch->clan ? wch->clan->who_name : "       ",
		        wch->act_flags.has(PLR_MAKEBAG)       ? "{CH{x" : "{T-{x",
		        wch->pcdata->plr_flags.has(PLR_OOC)   ? "{YR{x" : "{b-{x",
		        wch->pcdata->plr_flags.has(PLR_PAINT) ? "{V*{x" : "{M-{x");

		/* Format it up. */
		/* new format -- Elrac */
		if (wch->act_flags.has(PLR_SUPERWIZ)) {
			lbrk = "{P({R";
			rbrk = "{P)";
		}
		else if (wch->invis_level && wch->lurk_level) {
			lbrk = "{T{{";
			rbrk = "{T}";
		}
		else if (wch->invis_level) {
			lbrk = "{H<";
			rbrk = "{H>";
		}
		else if (wch->lurk_level) {
			lbrk = "{G(";
			rbrk = "{G)";
		}
		else {
			lbrk = "{W";
			rbrk = "";
		}

		Format::sprintf(buf, "%s %s ", block1, block2);

		if (wch->comm_flags.has(COMM_AFK))        buf += "{b[AFK]{x ";

		if (wch->act_flags.has(PLR_KILLER))       buf += "{R(KILLER){x ";

		if (wch->act_flags.has(PLR_THIEF))        buf += "{B(THIEF){x ";

		buf += lbrk;
		buf += wch->name;
		buf += rbrk;
		buf += "{x";
		buf += wch->pcdata->title;
		buf += "{x{a{W \n";
		output += buf;
	}

	stc("\n                             {C*{W*{C* {BL{Ce{gg{Wa{Cc{By {C*{W*{C*{x\n\n", ch);
	Format::sprintf(buf2, "\n");
	output += buf2;
	output += count_players(ch);
	output += "\n";
	set_color(ch, WHITE, BOLD);
	page_to_char(output, ch);
	set_color(ch, WHITE, NOBOLD);

} /* do_who() */

/* Short Who by Lotus */
void do_swho(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	String roombuf, output;
	Descriptor *d;
	output += "{WRP PK NH PB QW SQ NT  Name          Pos'n   Room Name{x\n";
	output += "{W----------------------------------------------------------------------------{x\n";

	for (d = descriptor_list; d != nullptr; d = d->next) {
		Character *wch;
		char const *position = "Drooling";

		if (!d->is_playing() || !can_see_who(ch, d->character))
			continue;

		wch   = (d->original != nullptr) ? d->original : d->character;

		if (!can_see_who(ch, wch))
			continue;

		switch (get_position(wch)) {
		case POS_DEAD:     position = "{WDead!";              break;

		case POS_MORTAL:   position = "{WWound";              break;

		case POS_INCAP:    position = "{WIncap";              break;

		case POS_STUNNED:  position = "{WStun ";              break;

		case POS_SLEEPING: position = "{gSleep";              break;

		case POS_RESTING:  position = "{bRest ";              break;

		case POS_SITTING:  position =
//			        (wch->on && wch->on->pIndexData->item_type == ITEM_COACH) ? "{YRide " : 
				"Sit  ";    break;

		case POS_FIGHTING: position = "{WFight";              break;

		case POS_STANDING: position = "Stand";                break;
		case POS_FLYING:   position = "Fly  ";                break;
		}

		/* Imms can now see rooms of privated mortals -- Elrac */
		if (!wch->pcdata->plr_flags.has(PLR_PRIVATE)) {
			roombuf = wch->in_room->name();

			while (roombuf.uncolor().size() > 37)
				roombuf.pop_back();
		}
		else if (!IS_IMMORTAL(ch) || IS_IMMORTAL(wch))
			Format::sprintf(roombuf, "Private");
		else {
			roombuf = Format::format("~%s", wch->in_room->name());

			while (roombuf.uncolor().size() > 36)
				roombuf.pop_back();

			roombuf += "{V~";
		}

		Format::sprintf(buf, "{b[%s][%s][%s][%s][%s][%s][%s{b] {W%-12s {R({P%s{R) {M<{V%s{M>\n",
		        wch->pcdata->plr_flags.has(PLR_OOC) ? "{Y*{b" : " ",
		        wch->pcdata->plr_flags.has(PLR_PK) ? "{G*{b" : " ",
		        wch->act_flags.has(PLR_MAKEBAG) ? "{T*{b" : " ",
		        wch->pcdata->plr_flags.has(PLR_PAINT) ? "{P*{b" : " ",
		        IS_QUESTOR(wch)                         ? "{V*{b" : " ",
		        IS_SQUESTOR(wch)                        ? "{B*{b" : " ",
		        (wch->pnote != nullptr) ?
		        wch->pnote->type == NOTE_NOTE           ? "{P*" :
		        wch->pnote->type == NOTE_IDEA           ? "{Y*" :
		        wch->pnote->type == NOTE_ROLEPLAY       ? "{V*" :
		        wch->pnote->type == NOTE_IMMQUEST       ? "{B*" :
		        wch->pnote->type == NOTE_CHANGES        ? "{G*" :
		        wch->pnote->type == NOTE_PERSONAL       ? "{C*" :
		        wch->pnote->type == NOTE_TRADE          ? "{b*" :
		        "{c?" :
		        " ",
		        wch->name,
		        position,
		        roombuf);
		output += buf;
	} /* end for */

	output += "{W----------------------------------------------------------------------------{x\n";
	page_to_char(output, ch);
} /* end do_swho() */

/* New container capable inventory - Lotus */
void do_inventory(Character *ch, String argument)
{
	Character *victim = nullptr;

	if (argument.empty()) {
		victim = ch;
		new_color(ch, CSLOT_MISC_INV);
		stc("You are carrying:\n", ch);
	}
	else {
		String arg1;
		argument = one_argument(argument, arg1);

		if (arg1 == "char" && IS_IMMORTAL(ch)) {
			victim = get_char_world(ch, argument, VIS_PLR);

			if (victim == nullptr) {
				stc("They aren't in the game.\n", ch);
				return;
			}

			new_color(ch, CSLOT_MISC_INV);
			act("$N is carrying:", ch, nullptr, victim, TO_CHAR, POS_SLEEPING, false);
		}
		else if (arg1 == "all") {
			victim = ch;
			new_color(ch, CSLOT_MISC_INV);
			stc("You are carrying:\n", ch);
			show_list_to_char(victim->carrying, ch, true, true, true);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
		else {
			stc("Syntax: Inventory or Inventory all\n", ch);

			if (IS_IMMORTAL(ch))
				stc("      : Inventory char <victim>\n", ch);

			return;
		}
	}

	show_list_to_char(victim->carrying, ch, true, true, false);
	set_color(ch, WHITE, NOBOLD);
}

void do_equipment(Character *ch, String argument)
{
	Object *obj;
	int iWear;
	String buf;
	Character *victim;

	if (IS_IMMORTAL(ch) && !argument.empty()) {
		if ((victim = get_player_world(ch, argument, VIS_PLR)) == nullptr)
			if ((victim = get_char_world(ch, argument, VIS_PLR)) == nullptr) {
				stc("They're not in the game.\n", ch);
				return;
			}

		set_color(ch, GREEN, NOBOLD);
		act("$N is using:", ch, nullptr, victim, TO_CHAR, POS_SLEEPING, false);
	}
	else {
		victim = ch;
		set_color(ch, GREEN, NOBOLD);
		stc("You are using:\n", ch);
	}

	for (iWear = 0; iWear < MAX_WEAR; iWear++) {
		if (ch->pcdata)
			if (iWear == WEAR_WEDDINGRING && !ch->pcdata->spouse.empty() && !IS_IMMORTAL(ch))
				continue;

		buf = where_name[iWear];

		if ((obj = get_eq_char(victim, iWear)) == nullptr) {
			if (!IS_IMMORTAL(victim)) {
				/* mortals can not hold shield & held AND secondary wpn */
				if ((iWear == WEAR_SHIELD || iWear == WEAR_HOLD)
				    && get_eq_char(victim, WEAR_SECONDARY) != nullptr)
					continue;

				if (iWear == WEAR_SECONDARY)
					continue;
			}

			buf += "       ({W- - -{x)";
		}
		else if (can_see_obj(ch, obj)) {
			int spaces;

			if (obj->num_settings == 0)
				spaces = MAX_GEM_SETTINGS + 3;
			else
				spaces = (MAX_GEM_SETTINGS - obj->num_settings);

			while (spaces-- > 0)
				buf += " ";

			buf += format_obj_to_char(obj, ch, true);
		}
		else
			buf += "       (something)";

		buf += "\n";
		stc(buf, ch);
	}

	set_color(ch, WHITE, NOBOLD);
}

void do_compare(Character *ch, String argument)
{
	Object *obj1;
	Object *obj2;
	int value1;
	int value2;
	char *msg;

	if (argument.empty()) {
		stc("Compare what to what?\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ((obj1 = get_obj_carry(ch, arg1)) == nullptr) {
		stc("You do not have one in your inventory.\n", ch);
		return;
	}

	if (arg2.empty()) {
		for (obj2 = ch->carrying; obj2 != nullptr; obj2 = obj2->next_content) {
			if (obj2->wear_loc != WEAR_NONE
			    &&  can_see_obj(ch, obj2)
			    && (obj1->wear_flags - ITEM_TAKE).has_any_of(obj2->wear_flags)) // any bits in common other than take?
				break;
		}

		if (obj2 == nullptr) {
			stc("You aren't wearing anything comparable.\n", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch, arg2)) == nullptr) {
		stc("You do not have one in your inventory.\n", ch);
		return;
	}

	msg         = nullptr;
	value1      = 0;
	value2      = 0;

	if (obj1 == obj2)
		msg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->item_type != obj2->item_type)
		msg = "You can't compare $p and $P.";
	else {
		switch (obj1->item_type) {
		default:
			msg = "You can't compare $p and $P.";
			break;

		case ITEM_ARMOR:
			value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
			value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
			break;

		case ITEM_WEAPON:
			value1 = (1 + obj1->value[2]) * obj1->value[1];
			value2 = (1 + obj2->value[2]) * obj2->value[1];
			break;
		}
	}

	if (msg == nullptr) {
		if (value1 == value2) msg = "$p and $P look about the same.";
		else if (value1  > value2) msg = "$p looks better than $P.";
		else                         msg = "$p looks worse than $P.";
	}

	act(msg, ch, obj1, obj2, TO_CHAR);
	return;
}

void do_credits(Character *ch, String argument)
{
	help(ch, "diku");
	help(ch, "credit");
	return;
}

void do_where(Character *ch, String argument)
{
	Duel::Arena *arena = arena_table_head->next;
	Descriptor *d;
	bool found = false;

	if (ch->in_room == nullptr)
		return;

	while (arena != arena_table_tail) {
		if (ch->in_room->prototype.vnum >= arena->minvnum
		    && ch->in_room->prototype.vnum <= arena->maxvnum)
			break;

		arena = arena->next;
	}

	if (arena != arena_table_tail) {
		for (auto victim : Game::world().char_list) {
			if (victim->in_room != nullptr
			    && victim->in_room->prototype.vnum >= arena->minvnum
			    && victim->in_room->prototype.vnum <= arena->maxvnum
			    && !victim->in_room->flags().has(ROOM_NOWHERE)
			    && can_see_char(ch, victim)) {
				found = true;
				ptc(ch, "%-28s{x %s{x\n", PERS(victim, ch, VIS_CHAR), victim->in_room->name());
			}
		}

		if (!found)
			stc("You can't see anyone nearby...\n", ch);

		return;
	}

	if (argument.empty()) {
		stc("Nearby you see:\n", ch);

		for (d = descriptor_list; d; d = d->next) {
			Character *victim = d->character;

			if (d->is_playing()
			    && victim->in_room != nullptr
			    && !victim->in_room->flags().has(ROOM_NOWHERE)
			    && victim->in_room->area() == ch->in_room->area()
			    && can_see_char(ch, victim)) {
				found = true;
				ptc(ch, "%-28s{x %s{x\n", victim->name, victim->in_room->name());
			}
		}

		if (!found)
			stc("None.\n", ch);
	}
	else {
		String arg;
		one_argument(argument, arg);

		for (auto victim : Game::world().char_list) {
			if (victim->in_room != nullptr
			    && victim->in_room->area() == ch->in_room->area()
			    && !affect::exists_on_char(victim, affect::type::hide)
			    && can_see_char(ch, victim)
			    && victim->name.has_words(arg)) {
				found = true;
				ptc(ch, "%-28s{x %s{x\n", PERS(victim, ch, VIS_CHAR), victim->in_room->name());
				break;
			}
		}

		if (!found)
			act("You didn't find any $T.", ch, nullptr, arg, TO_CHAR);
	}
}

/* New short consider by Lotus */
void do_scon(Character *ch, String argument)
{
	Character *victim;

	String arg;
	argument = one_argument(argument, arg);

	if (argument.empty()) {
		if ((victim = ch->fighting) == nullptr) {
			stc("Which way did he go?!?\n", ch);
			return;
		}
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They're not here.\n", ch);
		return;
	}

	ptc(ch, "{GN{H[{G%s{H] {CL{T[{C%d{T]\n"
	    "{CHp{T[{C%d{T/{C%d{T] {GMa{H[{G%d{H/{G%d{H] {BSt{N[{B%d{N/{B%d{N]{x\n",
	    victim->name, victim->level,
	    victim->hit, GET_MAX_HIT(victim),
	    victim->mana, GET_MAX_MANA(victim),
	    victim->stam, GET_MAX_STAM(victim));

	if (!strcmp(argument, "more")) {
		set_color(ch, WHITE, BOLD);
		ptc(ch, "Align[%d] Hit[%d] Dam[%d] AC[%d %d %d %d]\n",
		    victim->alignment, GET_ATTR_HITROLL(victim), GET_ATTR_DAMROLL(victim),
		    GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
		    GET_AC(victim, AC_SLASH),  GET_AC(victim, AC_EXOTIC));

		String buf;
		buf = print_defense_modifiers(victim, TO_ABSORB);
		if (!buf.empty()) ptc(ch, " Drain:  %s\n", buf);
		buf = print_defense_modifiers(victim, TO_IMMUNE);
		if (!buf.empty()) ptc(ch, " Immune: %s\n", buf);
		buf = print_defense_modifiers(victim, TO_RESIST);
		if (!buf.empty()) ptc(ch, " Resist: %s\n", buf);
		buf = print_defense_modifiers(victim, TO_VULN);
		if (!buf.empty()) ptc(ch, " Vuln:   %s\n", buf);
		buf = affect::print_cache(victim);
		if (!buf.empty()) ptc(ch, " Affect:  %s\n", buf);

		set_color(ch, WHITE, NOBOLD);
	}
}

void do_consider(Character *ch, String argument)
{
	String buf;
	Character *victim;
	int diff, percent;

	if (argument.empty()) {
		stc("Consider killing whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They're not here.\n", ch);
		return;
	}

	String guild = victim->is_npc() ? "mobile" :
	               victim->guild == Guild::none ? "none" :
	               guild_table[victim->guild].name;

	/* New Consider by Clerve */
	if (ch->has_cgroup(GROUP_AVATAR)) {
		ptc(ch, "{CYou see %s (level %d)\n", victim->name, victim->level);
		ptc(ch, "{TRace: %s  Sex: %s  Class: %s  Size: %s\n",
		    race_table[victim->race].name,
		    sex_table[GET_ATTR_SEX(victim)].name,
		    guild,
		    size_table[victim->size].name);
		ptc(ch, "{PStr: %-2d(%-2d)\t{BAC Pierce : %-10d{YHit Points: %d/%d\n",
		    ATTR_BASE(victim, APPLY_STR), GET_ATTR_STR(victim),
		    GET_AC(victim, AC_PIERCE), victim->hit, GET_MAX_HIT(victim));
		ptc(ch, "{PInt: %-2d(%-2d)\t{BAC Bash   : %-10d{YMana      : %d/%d\n",
		    ATTR_BASE(victim, APPLY_INT), GET_ATTR_INT(victim),
		    GET_AC(victim, AC_BASH), victim->mana, GET_MAX_MANA(victim));
		ptc(ch, "{PWis: %-2d(%-2d)\t{BAC Slash  : %-10d{YStamina   : %d/%d\n",
		    ATTR_BASE(victim, APPLY_WIS), GET_ATTR_WIS(victim),
		    GET_AC(victim, AC_SLASH), victim->stam, GET_MAX_STAM(victim));
		ptc(ch, "{PDex: %-2d(%-2d)\t{BAC Magic  : %-10d{GHit Roll  : %d\n",
		    ATTR_BASE(victim, APPLY_DEX), GET_ATTR_DEX(victim),
		    GET_AC(victim, AC_EXOTIC), GET_ATTR_HITROLL(victim));
		ptc(ch, "{PCon: %-2d(%-2d)\t\t\t      {GDam Roll  : %d\n",
		    ATTR_BASE(victim, APPLY_CON), GET_ATTR_CON(victim),
		    GET_ATTR_DAMROLL(victim));
		ptc(ch, "{PChr: %-2d(%-2d)\t{WSaves     : %-10dAlignment : %d\n",
		    ATTR_BASE(victim, APPLY_CHR), GET_ATTR_CHR(victim),
		    GET_ATTR_SAVES(victim), victim->alignment);

		if (victim->is_npc()) {
			ptc(ch, "\t\t{WDamage    : %2dd%-2d     Message   : %s\n",
			    victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
			    attack_table[victim->dam_type].noun);
			ptc(ch, "\t\t{WCount     : %-10dKilled    : %d\n",
			    victim->pIndexData->count, victim->pIndexData->killed);
		}

		ptc(ch, "\n{gAct: %s\n", act_bit_name(victim->act_flags, victim->is_npc()));

		if (!victim->is_npc())
			ptc(ch, "{gPlr: %s\n", plr_bit_name(victim->pcdata->plr_flags));

		if (victim->is_npc() && !victim->off_flags.empty())
			ptc(ch, "{gOff: %s\n", off_bit_name(victim->off_flags));

		buf = print_defense_modifiers(victim, TO_ABSORB);
		if (!buf.empty()) ptc(ch, "Drain:  %s\n", buf);
		buf = print_defense_modifiers(victim, TO_IMMUNE);
		if (!buf.empty()) ptc(ch, "Immune: %s\n", buf);
		buf = print_defense_modifiers(victim, TO_RESIST);
		if (!buf.empty()) ptc(ch, "Resist: %s\n", buf);
		buf = print_defense_modifiers(victim, TO_VULN);
		if (!buf.empty()) ptc(ch, "Vuln:   %s\n", buf);
		buf = affect::print_cache(victim);
		if (!buf.empty()) ptc(ch, "Affect: %s\n", buf);

		ptc(ch, "{gForm: %s\n{gParts: %s\n",
		    form_bit_name(victim->form_flags), part_bit_name(victim->parts_flags));
		ptc(ch, "{gMaster: %s  {gLeader: %s  {gPet: %s\n",
		    victim->master ? victim->master->name : "(none)",
		    victim->leader ? victim->leader->name : "(none)",
		    victim->pet   ? victim->pet->name   : "(none)");

		if (victim->is_npc() && victim->spec_fun != 0)
			ptc(ch, "{gMobile has special procedure %s.\n", spec_name(victim->spec_fun));

		for (const affect::Affect *paf = affect::list_char(victim); paf != nullptr; paf = paf->next) {
			if (paf->permanent)
				continue;

			ptc(ch, "{bSpell: '%s'", affect::lookup(paf->type).name);

			if (paf->where == TO_AFFECTS) {
				if (paf->location != APPLY_NONE && paf->modifier != 0)
					ptc(ch, " modifies %s by %d", affect_loc_name(paf->location), paf->modifier);
			}
			else if (paf->where == TO_DEFENSE)
				ptc(ch, " %s damage from %s by %d%%",
					paf->modifier > 0 ? "reduces" : "increases",
					dam_type_name(paf->location),
					paf->modifier > 0 ? paf->modifier : -paf->modifier);
			else
				stc(" does something weird", ch);

			if (paf->duration >= 0)
				ptc(ch, " for %d hours", paf->duration);

			ptc(ch, ", level %d", paf->level);

			if (paf->evolution > 1)
				ptc(ch, ", evolve %d", paf->evolution);

			stc(".\n", ch);
		}
	}

	diff = victim->level - ch->level;

	     if (diff <= -10)   buf = "You can kill $N naked and weaponless.";
	else if (diff <=  -5)   buf = "$N is no match for you.";
	else if (diff <=  -2)   buf = "$N looks like an easy kill.";
	else if (diff <=   1)   buf = "The perfect match!";
	else if (diff <=   4)   buf = "$N says 'Do you feel lucky, punk?'.";
	else if (diff <=   9)   buf = "$N laughs at you mercilessly..";
	else if (diff <=  30)   buf = "Death will thank you for your gift.";
	else                    buf = "$N could slay you in one blow.";

	set_color(ch, WHITE, BOLD);
	act(buf, ch, nullptr, victim, TO_CHAR);
	set_color(ch, WHITE, NOBOLD);

	if (GET_MAX_HIT(victim) > 0)
		percent = (100 * victim->hit) / GET_MAX_HIT(victim);
	else
		percent = -1;

	     if (percent >= 100) buf = "$N is in excellent condition.";
	else if (percent >=  90) buf = "$N has a few scratches.";
	else if (percent >=  75) buf = "$N has some small wounds and bruises.";
	else if (percent >=  50) buf = "$N has quite a few wounds.";
	else if (percent >=  30) buf = "$N has some big nasty wounds and scratches.";
	else if (percent >=  15) buf = "$N looks pretty hurt.";
	else if (percent >=   1) buf = "$N is in awful condition.";
	else if (percent >=   0) buf = "$N will soon be toast!!!";
	else                     buf = "$N is in need of ***SERIOUS*** medical attention!!!";

	act(buf, ch, nullptr, victim, TO_CHAR);
	set_color(ch, WHITE, NOBOLD);
}

void set_title(Character *ch, const String& title)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->is_npc()) {
		Logging::bug("Set_title: NPC.", 0);
		return;
	}

	if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?') {
		buf[0] = ' ';
		strcpy(buf + 1, title);
	}
	else
		strcpy(buf, title);

	ch->pcdata->title = buf;
	return;
}

void do_title(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (argument.empty()) {
		stc("Change your title to what?\n", ch);
		return;
	}

	if (argument == "none") {
		set_title(ch, "");
		stc("Title removed.\n", ch);
		return;
	}

	if (argument.uncolor().size() > 45) {
		stc("Titles can't be longer than 45 printed characters.\n", ch);
		return;
	}

	set_title(ch, argument);
	ptc(ch, "Title changed to: %s\n", ch->pcdata->title);
}

/*
 * Immname by Demonfire - Modified by Lotus
 * Added 11.25.1996
 * moved to config function -- Montrey
 */

void do_immname(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	char buf[MIL];
	Format::sprintf(buf, "immortal immname %s", argument);
	do_config(ch, buf);
}

void do_description(Character *ch, String argument)
{
	String buf;

	if (!argument.empty()) {
		if (argument[0] == '-') {
			int len;
			bool found = false;

			if (ch->description.empty()) {
				stc("No lines left to remove.\n", ch);
				return;
			}

			buf = ch->description;

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\n') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = true;
					}
					else { /* found the second one */
						buf.erase(len + 1);
						ch->description = buf;
						set_color(ch, PURPLE, NOBOLD);
						stc("Your description is:\n", ch);
						stc(ch->description.empty() ? "(None).\n" : ch->description, ch);
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf.clear();
			ch->description = buf;
			stc("Description cleared.\n", ch);
			return;
		}

		if (argument[0] == '+') {
			buf += ch->description;
			argument = argument.substr(1).lstrip();
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("Description too long.\n", ch);
			return;
		}

		buf += argument;
		buf += "\n";
		ch->description = buf;
	}

	set_color(ch, PURPLE, NOBOLD);
	stc("Your description is:\n", ch);
	stc(ch->description.empty() ? "(None).\n" : ch->description, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

/* Duplicate of Description for Finger Info */
void do_fingerinfo(Character *ch, String argument)
{
	String buf;

	if (ch->is_npc())
		return;

	if (!argument.empty()) {
		if (argument == "clear") {
			ch->pcdata->fingerinfo.erase();
			stc("Fingerinfo cleared.\n", ch);
			return;
		}
		else if (argument[0] == '-') {
			int len;
			bool found = false;

			if (ch->pcdata->fingerinfo.empty()) {
				stc("No lines left to remove.\n", ch);
				return;
			}

			buf = ch->pcdata->fingerinfo;

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\n') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = true;
					}
					else { /* found the second one */
						buf.erase(len + 1);
						ch->pcdata->fingerinfo = buf;
						set_color(ch, CYAN, NOBOLD);
						ptc(ch, "Your finger info is:\n%s",
						    ch->pcdata->fingerinfo.empty() ?
						    "(None).\n" : ch->pcdata->fingerinfo);
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf.clear();
			ch->pcdata->fingerinfo.erase();
			stc("Finger Info cleared.\n", ch);
		}
		else if (argument[0] == '+') {
			buf += ch->pcdata->fingerinfo;
			argument = argument.substr(1).lstrip();
		}
		else {
			stc("Syntax:\n"
			    "  fingerinfo           (shows your fingerinfo)\n"
			    "  fingerinfo -         (removes a line of text)\n"
			    "  fingerinfo + <text>  (adds a line of text)\n"
			    "  fingerinfo clear     (clears your fingerinfo)\n", ch);
			return;
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("Finger info too long.\n", ch);
			return;
		}

		buf += argument;
		buf += "\n";
		ch->pcdata->fingerinfo = buf;
	}

	set_color(ch, CYAN, NOBOLD);
	ptc(ch, "Your finger info is:\n"
	    "%s",
	    ch->pcdata->fingerinfo.empty() ?
	    "(None).\n" :
	    ch->pcdata->fingerinfo);
	set_color(ch, WHITE, NOBOLD);
}

void do_report(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];

	String arg;
	one_argument(argument, arg);

	ptc(ch,
	        "You say 'I have %d/%d hp %d/%d mana %d/%d st %d xp.'\n",
	        ch->hit,  GET_MAX_HIT(ch),
	        ch->mana, GET_MAX_MANA(ch),
	        ch->stam, GET_MAX_STAM(ch),
	        ch->exp);
	Format::sprintf(buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d st %d xp.'",
	        ch->hit,  GET_MAX_HIT(ch),
	        ch->mana, GET_MAX_MANA(ch),
	        ch->stam, GET_MAX_STAM(ch),
	        ch->exp);
	act(buf, ch, nullptr, nullptr, TO_ROOM);

	if (!strcmp(arg, "all")) {
		if (affect::list_char(ch) != nullptr) {
			stc("You say 'I am affected by the following spells:'\n", ch);
			act("$n says 'I am affected by the following spells:'", ch, nullptr, nullptr, TO_ROOM);

			const affect::Affect *paf_last = nullptr;
			for (const affect::Affect *paf = affect::list_char(ch); paf != nullptr; paf = paf->next) {
				if (paf_last != nullptr && paf->type == paf_last->type)
					continue;

				ptc(ch, "You say 'Spell: %-15s'\n", affect::lookup(paf->type).name);
				Format::sprintf(buf, "$n says 'Spell: %-15s'", affect::lookup(paf->type).name);
				act(buf, ch, nullptr, nullptr, TO_ROOM);
			}
		}
		else {
			stc("You say 'I am not affected by any spells.'\n", ch);
			act("$n says 'I am not affected by any spells.'", ch, nullptr, nullptr, TO_ROOM);
		}
	}

	return;
}

/* PRACTICE list of skills and spells, sorted by group -- Elrac */
void prac_by_group(Character *ch, const String& argument)
{
	const struct group_type *gp;
	bool group_first;
	skill::type sn;
	char buf[50];
	String line;
	int line_cols = 0;  /* number of filled data columns (19 char each) */
	String output;

	for (unsigned int gt = 0; gt < group_table.size(); gt++) { /* loop thru groups */
		if (!ch->pcdata->group_known[gt])   /* ignore groups the player doesn't have */
			continue;

		gp = &group_table[gt];

		if (gp->rating[ch->guild] < 0)      /* ignore things not in class */
			continue;

		if (!argument.empty() && !argument.is_prefix_of(gp->name))
			continue;

		group_first = true;

		for (unsigned int js = 0; js < gp->spells.size(); js++) { /* loop thru skills/spells */
			sn = skill::lookup(gp->spells[js]);

			if (sn == skill::type::unknown)
				continue;

			if (get_usable_level(sn, ch->guild) > ch->level)
				continue; /* skill beyond player's level */

			int learned = get_learned(ch, sn);

			if (learned <= 0)
				continue;

			/* finally, a skill he knows. */
			if (group_first) {  /* first in group get a new header */
				Format::sprintf(buf, "%s:", gp->name);
				/* Header, like all columns, formatted as 25 char + 1 sp */
				Format::sprintf(line, "{G%-25.25s{x ", buf);
				output += line;
				output += "\n";
				line.erase();
				line_cols = 0;
				group_first = false;
			}

			if (line_cols >= 3) {
				output += line;
				output += "\n";
				line.erase();
				line_cols = 0;
			}

			Format::sprintf(buf, "%3d%% %-20.20s ", learned,
			        gp->spells[js]);
			line += buf;
			line_cols++;
		} /* end for skills */

		if (line_cols > 0) { /* print last incomplete line */
			output += line;
			output += "\n";
			line.erase();
			line_cols = 0;
		}
	} /* end for groups */

	Format::sprintf(buf, "You have %d practice sessions left.\n", ch->practice);
	output += buf;
	page_to_char(output, ch);
} /* end prac_by_group() */

/* PRACTICE list of skills and spells, sorted by percentage and/or name -- Elrac */
void prac_by_key(Character *ch, const String& key, const char *argument)
{
	skill::type slist[skill::num_skills()];
	int nskills = 0;
	int js;
	int ip;
	int adept;
	char buf[50];
	int line_cols = 0;  /* number of filled data columns (19 char each) */
	String output;

	String arg;
	argument = one_argument(argument, arg);

	/* loop thru all skills */
	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		if (entry.remort_guild != Guild::none)
			if (!CAN_USE_RSKILL(ch, type))
				continue;

		if (get_usable_level(type, ch->guild) > ch->level)
			continue; /* skill beyond player's level */

		if (get_learned(ch, type) <= 0)
			continue; /* player doesn't know skill */

		if (!arg.empty() && !arg.is_prefix_of(entry.name))
			continue;

		/* skill is known. Insertion sort into slist by appropriate key. */
		ip = nskills;

		if (key[0] == '%') {
			while (ip > 0 &&
			       (get_learned(ch, slist[ip - 1]) <
			        get_learned(ch, type) ||
			        (get_learned(ch, slist[ip - 1]) ==
			         get_learned(ch, type) &&
			         strcmp(skill::lookup(slist[ip - 1]).name,
			                entry.name) > 0))) {
				slist[ip] = slist[ip - 1];
				ip--;
			}
		}
		else {
			while (ip > 0 && strcmp(skill::lookup(slist[ip - 1]).name,
			                        entry.name) > 0) {
				slist[ip] = slist[ip - 1];
				ip--;
			}
		}

		slist[ip] = type;
		nskills++;
	}

	/* slist has a sorted list of skills. Output in columns. */
	line_cols = 0;

	for (js = 0; js < nskills; js++) {
		const auto& entry = skill::lookup(slist[js]);

		if (entry.remort_guild != Guild::none) {
			if (entry.remort_guild == ch->guild)
				adept = 65;
			else
				adept = 50;
		}
		else
			adept = guild_table[ch->guild].skill_adept;

		output += Format::format("%s%3d%% %-20.20s{x ",
		    get_learned(ch, slist[js]) >= adept ? "{g" : "{C",
		    get_learned(ch, slist[js]),
		    entry.name);
		line_cols++;

		if (line_cols >= 3) {
			output += "\n";
			line_cols = 0;
		}
	}

	if (line_cols > 0)
		output += "\n";

	Format::sprintf(buf, "You have %d practice sessions left.\n", ch->practice);
	output += buf;
	page_to_char(output, ch);
} /* end prac_by_key() */

void do_practice(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *mob;
	int adept, increase;

	if (ch->is_npc()) {
		stc("You don't need to practice anything.\n", ch);
		return;
	}

	if (ch->guild == Guild::none) {
		stc("Join a guild if you want to learn some skills.\n", ch);
		return;
	}

	/*** PRACTICE (the info command) ***/
	String arg;
	const char *argtail = one_argument(argument, arg);

	if (arg.empty()) {
		prac_by_key(ch, "abc", "");
		return;
	}
	else if (arg.is_prefix_of("groups")) {
		prac_by_group(ch, argtail);
		return;
	}
	else if (arg.is_prefix_of("abc") ||
	         arg.is_prefix_of("%")) {
		prac_by_key(ch, arg, argtail);
		return;
	}

	/* none of the above returned, so it must be: */
	/*** PRACTICE <skill> ***/

	if (!IS_AWAKE(ch)) {
		stc("In your dreams, or what?\n", ch);
		return;
	}

	for (mob = ch->in_room->people; mob != nullptr; mob = mob->next_in_room) {
		if (mob->is_npc() && mob->act_flags.has(ACT_PRACTICE))
			break;
	}

	if (mob == nullptr) {
		stc("You can't do that here.\n", ch);
		return;
	}

	if (ch->practice <= 0) {
		stc("You have no practice sessions left.\n", ch);
		return;
	}

	skill::type sn = find_spell(ch, argument);
	const auto& entry = skill::lookup(sn);

	if (sn == skill::type::unknown
	    || (!ch->is_npc()
	        && (ch->level < get_usable_level(sn, ch->guild)
	            ||    get_learned(ch, sn) < 1 /* skill is not known */
	            ||    entry.rating[ch->guild] == 0))) {
		stc("You can't practice that.\n", ch);
		return;
	}

	if (entry.remort_guild != Guild::none)
		if (!CAN_USE_RSKILL(ch, sn)) {
			stc("You can't practice that.\n", ch);
			return;
		}

	adept = 0;

	if (entry.remort_guild != Guild::none) {
		if (entry.remort_guild == ch->guild)
			adept = 65;
		else if (HAS_EXTRACLASS(ch, sn))
			adept = 50;
	}
	else
		adept = ch->is_npc() ? 100 : guild_table[ch->guild].skill_adept;

	if (get_learned(ch, sn) >= adept) {
		Format::sprintf(buf, "You are already learned at %s.\n",
		        entry.name);
		stc(buf, ch);
		return;
	}

	ch->practice--;
	increase = (int_app[GET_ATTR_INT(ch)].learn / entry.rating[ch->guild]);

	if (increase < 1)
		increase = 1;

	set_learned(ch, sn, get_learned(ch, sn) + increase);

	if (get_learned(ch, sn) < adept) {
		act("You practice $T.",
		    ch, nullptr, entry.name, TO_CHAR);
		act("$n practices $T.",
		    ch, nullptr, entry.name, TO_ROOM);
	}
	else {
		set_learned(ch, sn, adept);
		act("You are now learned at $T.",
		    ch, nullptr, entry.name, TO_CHAR);
		act("$n is now learned at $T.",
		    ch, nullptr, entry.name, TO_ROOM);
	}
} /* end do_practice() */


/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	int wimpy;

	String arg;
	one_argument(argument, arg);

	if (arg.empty())
		wimpy = GET_MAX_HIT(ch) / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0) {
		stc("Your courage exceeds your wisdom.\n", ch);
		return;
	}

	if (wimpy > GET_MAX_HIT(ch) / 2) {
		stc("CHICKEN!!!!!\n", ch);
		return;
	}

	ch->wimpy   = wimpy;
	Format::sprintf(buf, "Wimpy set to %d hit points.\n", wimpy);
	stc(buf, ch);
	return;
}

void do_password(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 */
	String arg1, arg2;
	argument.lsplit(arg1).lsplit(arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax: password <old> <new>.\n", ch);
		return;
	}

	/*    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) ) */
	if (strcmp(arg1, ch->pcdata->pwd)) {
		WAIT_STATE(ch, 40);
		stc("Wrong password.  Wait 10 seconds.\n", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		stc(
		        "New password must be at least five characters long.\n", ch);
		return;
	}

	ch->pcdata->pwd = arg2;
	save_char_obj(ch);
	stc("Password Changed.\n", ch);
	return;
}
void do_invite(Character *ch, String argument)
{
	Character *victim;
	char buf[MAX_STRING_LENGTH];
	Descriptor *d;
	bool found = false;

	if (argument.empty()) {
		if (IS_IMMORTAL(ch)) {
			for (d = descriptor_list; d != nullptr; d = d->next) {
				if (!d->is_playing() || !can_see_who(ch, d->character))
					continue;

				victim = (d->original != nullptr) ? d->original : d->character;

				if (!can_see_who(ch, victim))
					continue;

				if (victim->inviters != nullptr) {
					if (!found) {
						stc("{M[{YName        {M][{YInviting Clan                 {M][{YStatus  {M]\n", ch);
						stc("--------------------------------------------------------{x\n", ch);
					}

					Format::sprintf(buf, "{M[{Y%-12s{M][{Y%-30s{M][{Y%-8s{M]{x\n", victim->name,
					        victim->inviters->clanname,
					        (victim->invitation_accepted == true) ? "accepted" : "invited");
					stc(buf, ch);
					found = true;
				}
			}

			if (!found)
				stc("There are currently no invited players.\n", ch);
		}
		else {
			if (!ch->has_cgroup(GROUP_LEADER | GROUP_DEPUTY)) {
				stc("Only leaders or deputies of your clan may send out invitations.\n", ch);
				return;
			}
			else
				stc("You must specify a name if you want to invite someone.\n", ch);
		}

		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1 == "terminate") {
		if (argument.empty()) {
			if ((ch->inviters != nullptr)) {
				stc("You have terminated your invitation.\n", ch);
				ch->inviters = nullptr;
				ch->invitation_accepted = false;
				return;
			}

			/* if ((ch->invite != 0) && (ch->invite >= INVITED))
			 {
			     stc("Please use invite decline.\n",ch);
			     return;
			 }
			 */
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if ((victim = get_char_world(ch, argument, VIS_PLR)) == nullptr) {
			stc("The person you want to uninvite is not here.\n", ch);
			return;
		}

		if (victim->inviters != nullptr) {
			if (victim->inviters == ch->clan) {
				stc("You have terminated their invitation.\n", ch);
				Format::sprintf(buf, "%s has terminated your invitation.\n", ch->name);
				victim->inviters = nullptr;
				victim->invitation_accepted = false;
				stc(buf, victim);
			}
			else
				stc("That player is not invited to your clan!\n", ch);
		}
		else {
			stc("That player is not invited to your clan!\n", ch);
			return;
		}

		stc("That player has not been invited.\n", ch);
		return;
	}

	if (!strcmp(arg1, "accept")) {
		if (ch->inviters == nullptr) {
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if (ch->invitation_accepted == true) {
			stc("You have already accepted the invitation!\n", ch);
			return;
		}

		if (!deduct_cost(ch, 10000)) { /* 100 gold to accept */
			stc("You must have at least 100 gold to accept an invitation.\n", ch);
			return;
		}

		stc("You have accepted their invitation.\n", ch);
		ch->invitation_accepted = true;

		for (d = descriptor_list; d != nullptr; d = d->next) {
			victim = d->original ? d->original : d->character;

			if (d->is_playing()
			    && d->character != ch
			    && ((ch->inviters == d->character->clan)
			        || IS_IMMORTAL(d->character))
			    && !d->character->comm_flags.has(COMM_NOCLAN)
			    && !d->character->comm_flags.has(COMM_QUIET)) {
				new_color(d->character, CSLOT_CHAN_CLAN);

				if (IS_IMMORTAL(d->character)) {
					Format::sprintf(buf, "%s",
					        ch->inviters->who_name.uncolor());
					stc(buf, d->character);
				}

				Format::sprintf(buf, "%s has accepted an invitation to enter the clan hall",
				        ch->name);
				act("Clan Notice: '$t{x'", ch, buf, d->character, TO_VICT, POS_DEAD, false);
				set_color(d->character, WHITE, NOBOLD);
			}
		}

		return;
	}

	if (!strcmp(arg1, "decline")) {
		if (ch->inviters == nullptr) {
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if (ch->invitation_accepted == true) {
			stc("You have already accepted the invitation!\n", ch);
			return;
		}

		stc("You have declined their invitation.\n", ch);

		for (d = descriptor_list; d != nullptr; d = d->next) {
			victim = d->original ? d->original : d->character;

			if (d->is_playing()
			    && d->character != ch
			    && ((ch->inviters == d->character->clan)
			        || IS_IMMORTAL(d->character))
			    && !d->character->comm_flags.has(COMM_NOCLAN)
			    && !d->character->comm_flags.has(COMM_QUIET)) {
				new_color(d->character, CSLOT_CHAN_CLAN);

				if (IS_IMMORTAL(d->character)) {
					Format::sprintf(buf, "%s",
					        ch->inviters->who_name.uncolor());
					stc(buf, d->character);
				}

				Format::sprintf(buf, "%s has declined an invitation to enter the clan hall",
				        ch->name);
				act("Clan Notice: '$t{x'", ch, buf, d->character, TO_VICT, POS_DEAD, false);
				set_color(d->character, WHITE, NOBOLD);
			}
		}

		ch->inviters = nullptr;
		ch->invitation_accepted = false;
		return;
	}

	if (!ch->has_cgroup(GROUP_LEADER | GROUP_DEPUTY)) {
		stc("Only leaders or deputies of your clan may send out invitations.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("The person you want to invite is not here.\n", ch);
		return;
	}

	if (victim->is_npc()) {
		stc("You cannot invite mobiles to your clan hall.\n", ch);
		return;
	}

	if (victim->inviters != nullptr) {
		if (victim->inviters == ch->clan)
			stc("They already have an invitation to your clan hall.\n", ch);
		else
			stc("They already have an invitation to a different clan hall.\n", ch);

		return;
	}

	if (victim == ch) {
		stc("Feeling lonely?\n", ch);
		return;
	}

	if (victim->clan == ch->clan) {
		stc("They don't need an invitation to enter your clan hall.\n", ch);
		return;
	}

	victim->inviters = ch->clan;
	victim->invitation_accepted = false;
	Format::sprintf(buf, "You have invited %s to enter your clan hall.\n", victim->name);
	stc(buf, ch);
	Format::sprintf(buf, "%s has invited you to enter the clan hall of %s.\n", ch->name,
	        ch->clan->clanname);
	stc(buf, victim);
} /* do_invite */

void do_join(Character *ch, String argument)
{
	Character *victim;

	if (IS_IMMORTAL(ch)) {
		stc("Use 'guild' instead.\n", ch);
		return;
	}

	if (!ch->clan || !ch->has_cgroup(GROUP_LEADER | GROUP_DEPUTY)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Whom do want to join your clan?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
		stc("They aren't playing.\n", ch);
		return;
	}

	if (victim->level < 15 && !IS_REMORT(victim)) {
		stc("They are too inexperienced to join your clan.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You cannot join immortals.\n", ch);
		return;
	}

	if (victim->clan) {
		stc("They're already in a clan.\n", ch);
		return;
	}

	if (get_duel(victim)) {
		stc("Wait until they finish their duel.\n", ch);
		return;
	}

	/* remove a leader flag, in case they were a leader of a clan that poofed */
	victim->remove_cgroup(GROUP_LEADER);
	victim->remove_cgroup(GROUP_DEPUTY);
	victim->add_cgroup(GROUP_CLAN);
	victim->clan = ch->clan;
	victim->pcdata->questpoints_donated = 0;
	victim->pcdata->gold_donated = 0;
	save_char_obj(victim);
	ptc(ch, "The character is now a member of %s.\n", ch->clan->clanname);
	ptc(victim, "You are now a member of %s.\n", ch->clan->clanname);
}

// new recursive function to vaporize clan equipment.  the idea is to start
// at the end of the contents list, then descend into contents, appending surviving
// equipment and returning it.
Object *vape_ceq_recursive(Character *ch, Object *obj, int depth) {
	if (obj == nullptr)
		return nullptr;

	// recurse to the end of the list
	obj->next_content = vape_ceq_recursive(ch, obj->next_content, depth);

	// go into the contents
	obj->contains = vape_ceq_recursive(ch, obj->contains, depth+1);

	// and gems (same depth, we can see them blow up if this is depth 0)
	obj->gems = vape_ceq_recursive(ch, obj->gems, depth);

	// and return this, if it's not clan eq
	if (obj->pIndexData->vnum < ch->clan->area_minvnum
	 || obj->pIndexData->vnum > ch->clan->area_maxvnum)
		return obj;

	// now destroy this object if necessary, and return the surviving contents and next_content
	if (depth > 0)
		stc("You hear a muffled {Yexplosion{x and smell {gsmoke{x from your belongings.\n\r", ch);
	else
		ptc(ch, "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n", obj->short_descr);

	// the contents and gems have already been checked, insert them in this content list
	Object *prev, *o, *o_next;

	// find the tail of the contents and parse them at the same time
	for (o = obj->contains, prev = nullptr; o; o = o_next) {
		o_next = o->next_content;

		// weird unlikely case of money going to player's inventory
		if (depth == 0 && o->item_type == ITEM_MONEY) {
			ch->silver += o->value[0];
			ch->gold += o->value[1];

			// handle the lists here, just destroy the thing
			if (prev)
				prev->next_content = o->next;

			o->in_room = nullptr;
			o->carried_by = nullptr;
			o->in_obj = nullptr;
			o->in_locker = nullptr;
			o->in_strongbox = nullptr;
			extract_obj(o);
			continue;
		}

		o->in_obj = obj->in_obj; // update owner
		prev = o;
	}

	// insert into list directly after this object
	if (prev) {
		prev->next_content = obj->next_content;
		obj->next_content = obj->contains;
		obj->contains = nullptr;
	}

	// do the same thing for gems, just finding the tail
	for (o = obj->gems, prev = nullptr; o; o = o_next) {
		o_next = o->next;
		o->in_obj = obj->in_obj; // update owner
		prev = o;
	}

	// insert into list directly after this object
	if (prev) {
		prev->next_content = obj->next_content;
		obj->next_content = obj->gems;
		obj->gems = nullptr;
	}

	// now obj->next_content is the object's former gems, then former contents,
	// then it's former next content.  we can now trash the item and return the remains
	Object *surviving = obj->next_content;

	// remove the object from the game, but we'll handle the lists here
	obj->in_room = nullptr;
	obj->carried_by = nullptr;
	obj->in_obj = nullptr;
	obj->in_locker = nullptr;
	obj->in_strongbox = nullptr;
	extract_obj(obj);
	return surviving;
}

void vape_ceq(Character *ch)
{
	ch->carrying = vape_ceq_recursive(ch, ch->carrying, 0);
}

void do_unjoin(Character *ch, String argument)
{
	Character *victim;
	Clan *clan;

	if (IS_IMMORTAL(ch)) {
		stc("Use 'guild' instead.\n", ch);
		return;
	}

	if (!ch->clan || !ch->has_cgroup(GROUP_LEADER)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Whom do want to remove from your clan?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
		stc("They aren't playing.\n", ch);
		return;
	}

	if (victim->clan != ch->clan) {
		stc("They're not in your clan.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You cannot unjoin immortals.\n", ch);
		return;
	}

	if (get_duel(victim)) {
		stc("Wait until they finish their duel.\n", ch);
		return;
	}

	if (victim->has_cgroup(GROUP_LEADER)) {
		stc("They must be deleadered first.\n", ch);
		return;
	}

	/* Get rid of their clan eq, twice for clanbags (put in a recursive form
	   later, this is stupid) -- Montrey */
	vape_ceq(victim);
//	vape_ceq(victim);   14 years later, done! -- Montrey

	ptc(ch, "The character is no longer a member of: %s\n", ch->clan->clanname);
	ptc(victim, "You are no longer a member of: %s\n", ch->clan->clanname);
	clan = victim->clan;
	victim->clan = nullptr;

	/* boot them from the clanhall */
	if (victim->in_room->prototype.vnum >= clan->area_minvnum
	    && victim->in_room->prototype.vnum <= clan->area_maxvnum) {
		char_from_room(victim);
		char_to_room(victim, Game::world().get_room(Location(Vnum(ROOM_VNUM_ALTAR))));
		do_look(ch, "auto");
		stc("You have been removed from the clanhall.\n", victim);
	}

	/* Remove Leader flag!!!! */
	victim->remove_cgroup(GROUP_LEADER);
	victim->remove_cgroup(GROUP_DEPUTY);
	victim->remove_cgroup(GROUP_CLAN);
	victim->pcdata->questpoints_donated = 0;
	victim->pcdata->gold_donated = 0;
	save_char_obj(victim);
}

/* Clan Status by Lotus */
/* Begin modification by Lotus and Slipstream */
void do_rank(Character *ch, String argument)
{
	char test[MAX_STRING_LENGTH];
	Character *victim;

	if (!ch->has_cgroup(GROUP_LEADER | GROUP_DEPUTY) && !IS_IMMORTAL(ch)) {
		stc("Sorry, only a leader can change clan rank.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Change whose clan rank?\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("The player is not logged on.\n", ch);
		return;
	}

	if (victim->clan != ch->clan && !IS_IMMORTAL(ch)) {
		stc("They are not a member of your clan.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Change their clan rank to what?\n"
		    "(use 'none' to remove rank)\n", ch);
		return;
	}

	if (argument.is_prefix_of("none")) {
		stc("Clan rank removed.\n", ch);
		stc("Your clan rank has been removed.\n", victim);
		victim->pcdata->rank.erase();
		return;
	}

	if (argument.uncolor().size() > 3) {
		stc("String cannot be longer than 3 printed characters.\n", ch);
		return;
	}

	if (strlen(argument) > 25) {
		stc("String cannot be longer than 25 total characters.\n", ch);
		return;
	}

	victim->pcdata->rank = argument.center(3);
	Format::sprintf(test, "Your new rank is {W[%s{W]{x.\n",
	        victim->pcdata->rank);
	stc(test, victim);
	stc("Clan rank changed.\n", ch);
	return;
}

/* prefix command: it will put the string typed on each line typed */
void do_prefi(Character *ch, String argument)
{
	stc("You cannot abbreviate the prefix command.\n", ch);
	return;
}

void do_prefix(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (argument.empty()) {
		if (ch->prefix.empty()) {
			stc("You have no prefix to clear.\n", ch);
			return;
		}

		stc("Prefix removed.\n", ch);
		ch->prefix.erase();
		return;
	}

	if (!ch->prefix.empty()) {
		Format::sprintf(buf, "Prefix changed to: %s.\n", argument);
	}
	else
		Format::sprintf(buf, "Prefix set to: %s.\n", argument);

	stc(buf, ch);
	ch->prefix = argument;
} /* end do_prefix() */

void do_email(Character *ch, String argument)
{
	char buf[MIL];

	if (ch->is_npc()) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		ptc(ch, "Your current email address is: %s\n", ch->pcdata->email);
		return;
	}

	strcpy(buf, argument);
	ch->pcdata->email = buf;

	ptc(ch, "Your email has been changed to: %s\n", buf);
	Format::sprintf(buf, "\"%s\" <%s>\n", ch->name, ch->pcdata->email);
	fappend(EMAIL_FILE, buf);
	Format::sprintf(buf, "$N has changed their email to '%s'\n", ch->pcdata->email);
	wiznet(buf, ch, nullptr, WIZ_MAIL, 0, GET_RANK(ch));
}

/* gameinout: show game entry or exit message -- Elrac */
void gameinout(Character *ch, const String& mortal, const String& entryexit, char inout)
{
	Character *victim;
	String *msgptr;
	char buf[MAX_INPUT_LENGTH];
	char *p;

	if (ch->is_npc()) {
		stc("Mobs don't have entry/exit messages!\n", ch);
		return;
	}

	if (mortal.empty())
		victim = ch;
	else {
		victim = get_player_world(ch, mortal, VIS_PLR);

		if (victim == nullptr) {
			act("No player named '$t' in the game!", ch, mortal, nullptr, TO_CHAR);
			return;
		}
	}

	if (inout == 'I')
		msgptr = &victim->pcdata->gamein;
	else
		msgptr = &victim->pcdata->gameout;

	if (msgptr->empty()) {
		if (victim == ch)
			act("You don't have a game $t message!",
			    ch, entryexit, nullptr, TO_CHAR);
		else
			act("$N does not have a game $t message!",
			    ch, entryexit, victim, TO_CHAR);

		return;
	}

	if (victim == ch)
		act(*msgptr, ch, nullptr, nullptr, TO_CHAR);
	else {
		/* convert $n to $N for 3rd person form of game msg */
		strcpy(buf, *msgptr);

		for (p = buf; *p; p++)
			if (*p == '$')
				switch (*(p + 1)) {
				case 'n': p[1] = 'N'; break;

				case 'e': p[1] = 'E'; break;

				case 'm': p[1] = 'M'; break;

				case 's': p[1] = 'S'; break;
				}

		act(buf, ch, nullptr, victim, TO_CHAR);
	}
} /* end gameinout() */

void do_gamein(Character *ch, String argument)
{
	if (!IS_IMMORTAL(ch) || argument.empty())
		gameinout(ch, "", "entry", 'I');
	else
		gameinout(ch, argument, "entry", 'I');
} /* end do_gamein() */

void do_gameout(Character *ch, String argument)
{
	if (!IS_IMMORTAL(ch) || argument.empty())
		gameinout(ch, "", "exit", 'O');
	else
		gameinout(ch, argument, "exit", 'O');
} /* end do_gameout() */

/* Show contents of the pit, selected by level -- Elrac */
void do_pit(Character *ch, String argument)
{
	Object *pit;
	Object sel_pit; /* a real live container-type object! */
	Object *obj, *next_obj;
	int num1 = -1, num2 = -1;
	int level1 = 0, level2 = 0;
	bool flevel = false, fexplevel = false, fname = false, fwear = false, fweapon = false;
	Flags wear_flag;
	int weapon_type = -1;

	if (!check_blind(ch))
		return;

	if ((pit = get_obj_here(ch, "pit")) == nullptr) {
		stc("You do not see a donation pit here.\n", ch);
		return;
	}

	if (pit != Game::world().donation_pit) {
		stc("You seem to be looking at the wrong pit.\n", ch);
		return;
	}

	if (pit->contains == nullptr) {
		stc("The pit is empty at the moment.\n", ch);
		return;
	}

	/* scan [ [level [level] ] keyword(s) ] */
	String keywords = argument;
	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty())
		keywords = "";
	else {
		if (arg.is_number()) {
			num1 = std::max(atoi(arg), -1);
			keywords = argument;
			argument = one_argument(argument, arg);

			if (arg.empty())
				keywords = "";
			else {
				if (arg.is_number()) {
					num2 = std::max(atoi(arg), -1);
					keywords = argument;
					one_argument(argument, arg);

					if (arg.empty())
						keywords = "";
				}
			}
		}
	}

	/* interpret num1 and num2 into a level range */
	fname = !keywords.empty();

	if (num1 == -1 && !fname) {
		/* no number, no keyword */
		flevel = true;
		level1 = std::min(ch->level - 7, 2 * ch->level / 3);
		level1 = std::max(level1, 0);
		level2 = get_holdable_level(ch);
	}
	else if (num1 == -1) {
		/* keyword only */
		flevel = false;
	}
	else if (num2 == -1) {
		/* level 1 but no level 2, maybe keyword */
		fexplevel = true;
		flevel = true;
		level1 = std::max(num1 - 10, 0);
		level2 = num1 + 10;
	}
	else {
		/* level 1 and level 2, maybe keyword */
		fexplevel = true;
		flevel = true;
		level1 = std::max(num1, 0);
		level2 = std::max(num2, level1);
	}

	if (keywords.has_prefix("wear")) {
	 	fwear = true;
	 	fname = false;
		keywords = one_argument(keywords, arg); // strip off the "wear "
		keywords = one_argument(keywords, arg); // get the wear slot name

		if (!arg.empty()) {
			int index = flag_index_lookup(arg, wear_flags); // gets the index

			if (index == -1) {
				stc("That is not a wear location you can search for in the pit.\n", ch);
				ptc(ch, "Searchable wear locations are:\n  %s", wear_bit_name(Flags::all));
				return;
			}

			wear_flag = wear_flags[index].bit; // gets the bit
			keywords = "items that can be worn there";
		}
		else {
			wear_flag = Flags(Flags::all) - (ITEM_TAKE | ITEM_NO_SAC | ITEM_WEAR_WEDDINGRING); // everything except
			keywords = "wearable items";
		}
	}
	else if (keywords.has_prefix("weapon")) {
		fweapon = true;
	 	fname = false;
		keywords = one_argument(keywords, arg); // strip off the "weapon "
		keywords = one_argument(keywords, arg); // get the weapon type name

		if (!arg.empty()) {
			int index = weapon_lookup(arg); // gets the index

			if (index == -1) {
				stc("That is not a weapon type you can search for in the pit.\n", ch);
				stc("Searchable weapon types are:\n  ", ch);
				for (const auto& entry : weapon_table)
					ptc(ch, "%s ", entry.name);
				stc("\n", ch);
				return;
			}

			weapon_type = weapon_table[index].type; // gets the bit
			keywords = "weapons of that type";
		}
		else {
			// otherwise weapon_type == -1, we'll list all weapons
			keywords = "weapons";
		}
	}

	/* hastily dummy up an empty clone of the pit */
	/* not a bona fide object, it is on local stack and not in obj list */
//	memset(&sel_pit, 0, sizeof(sel_pit));
	sel_pit.pIndexData = pit->pIndexData;
	clone_object(pit, &sel_pit);
	sel_pit.in_room = pit->in_room;

	/* select pit items into 'sel_pit' for looking at */
	for (obj = pit->contains; obj != nullptr; obj = next_obj) {
		next_obj = obj->next_content;

		if (flevel &&
		    (obj->level < level1 || obj->level > level2))
			continue;

		if (fname && !obj->name.has_words(keywords))
			continue;

		if (fwear && obj->wear_flags.has_none_of(wear_flag))
			continue;

		if (fweapon
		 && (obj->item_type != ITEM_WEAPON 
		  || (weapon_type != -1 && obj->value[0] != weapon_type)))
			continue;

		obj_from_obj(obj);
		obj_to_obj(obj, &sel_pit);
	}

	if (sel_pit.contains == nullptr) {
		ptc(ch, "You see no %s%s in the pit.\n",
			keywords,
			flevel ? fexplevel ? "around that level" : "around your level" : "");
	}
	else {
		/* code copied from do_look(), so should be convincing. */
		ch->act_flags += PLR_LOOKINPIT;
		new_color(ch, CSLOT_MISC_OBJECTS);
		act("In your selection from $p, you see:", ch, pit, nullptr, TO_CHAR);
		show_list_to_char(sel_pit.contains, ch, true, true, false);
		set_color(ch, WHITE, NOBOLD);
		ch->act_flags -= PLR_LOOKINPIT;
	}

	/* stuff goes back into pit. rummaging shuffles contents, that's ok */
	for (obj = sel_pit.contains; obj != nullptr; obj = next_obj) {
		next_obj = obj->next_content;
		obj_from_obj(obj);
		obj_to_obj(obj, pit);
	}
} /* end do_pit() */

/* Show information of a clan to a member */
void do_claninfo(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (ch->is_npc()) {
		stc("You just leave worrying about clans for players.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (ch->clan == nullptr) {
			stc("You're not even in a clan!\n", ch);
			return;
		}

		if (ch->clan->independent) {
			stc("Your clan doesn't support this feature!\n", ch);
			return;
		}

		Format::sprintf(buf, "You are a member of %s, created by %s.\n",
		        ch->clan->clanname, ch->clan->creator);
		stc(buf, ch);
		Format::sprintf(buf, "To send a note to your clan, send a note to '%s'.\n",
		        ch->clan->name);
		stc(buf, ch);
		Format::sprintf(buf, "The clan has %ld questpoints, of which %d came from you.\n",
		        ch->clan->clanqp, ch->pcdata->questpoints_donated);
		stc(buf, ch);
		Format::sprintf(buf, "The clan has %ld gold coins, of which %ld came from you.\n",
		        ch->clan->gold_balance, ch->pcdata->gold_donated);
		stc(buf, ch);
	}
	else {
		Clan *iterator;
		stc("Clanname            |QP     |Gold\n", ch);
		stc("--------------------+-------+------\n", ch);

		for (iterator = clan_table_head->next; iterator != clan_table_tail; iterator = iterator->next) {
			if (iterator->independent)
				continue;

			Format::sprintf(buf, "%-20s|%-7ld|%ld\n", iterator->name, iterator->clanqp, iterator->gold_balance);
			stc(buf, ch);
		}

		stc("--------------------+-------+------\n", ch);
	}

	return;
}

String make_bar(const String& bgcolor, const String& fgcolor, long info, int numbg, bool imm)
{
	String output, buf;

	if (imm)
		Format::sprintf(output, "{%s{%s%-10ld{x", bgcolor, fgcolor, info);
	else
		Format::sprintf(output, "{%s{%s          {x", bgcolor, fgcolor);

	Format::sprintf(buf, "{x{%s", fgcolor);
	return output.insert(buf, numbg + 4);
}

void do_clanpower(Character *ch, String argument)
{
	Clan *clan;
	int count = 0, inc, i, x, j = 0, high, low, negmod = 0, total;
	String output;
	struct c_list {
		Clan *clan;
		int members;
		int tmembers;
		int qp;
		int gold;
		int warpts;
		bool printed;
	};
	struct c_list clist[count_clans()];

	for (clan = clan_table_head->next; clan != clan_table_tail; clan = clan->next) {
		if (clan->independent)
			continue;

		clist[count].clan       = clan;
		clist[count].tmembers   = count_clan_members(clan, 0);
		clist[count].qp         = clan->clanqp;
		clist[count].gold       = clan->gold_balance;
		clist[count].warpts     = clan->warcpmod;
		clist[count].printed    = false;
		count++;
	}

	/* presorting, make each int a number between 1 and... eh, 10 */
	for (i = 1, high = clist[0].tmembers; i < count; i++)
		high = std::max(clist[i].tmembers, high);

	for (i = 0; i < count; i++)
		clist[i].members = high ? URANGE(1, ((9 * clist[i].tmembers) / high) + 1, 10) : 1;

	for (i = 1, high = clist[0].qp; i < count; i++)
		high = std::max(clist[i].qp, high);

	for (i = 0; i < count; i++)
		clist[i].qp = high ? URANGE(1, ((9 * clist[i].qp) / high) + 1, 10) : 1;

	for (i = 0; i < count; i++)
		clist[i].gold = URANGE(1, ((9 * clist[i].gold) / 200000) + 1, 10);

	/* war points can be negative! */
	for (i = 1, high = clist[0].warpts, low = clist[0].warpts; i < count; i++) {
		high = std::max(clist[i].warpts, high);
		low = std::min(clist[i].warpts, low);
	}

	if (low < 0)
		negmod = 0 - low;

	high += negmod;
	low += negmod;
	inc = high - low;

	for (i = 0; i < count; i++)
		clist[i].warpts = inc ? URANGE(1, ((9 * (clist[i].warpts + negmod - low)) / inc) + 1, 10) : high ? 10 : 1;

	output += "\n                            {WThe {BClans{W of {BL{Ce{gg{Wa{Cc{By{W:\n\n";
	output += Format::format("%s{CSize:      {GFame:      {YWealth:    {PMight:{x\n",
	    IS_IMMORTAL(ch) ? "                                       " :
	    "                                  ");

	for (i = 0; i < count; i++) {
		for (x = 0, high = 0; x < count; x++)
			if (!clist[x].printed) {
				total = calc_cp(clist[x].clan, true);

				if (total >= high) {
					j = x;
					high = total;
				}
			}

		if (IS_IMMORTAL(ch))
			output += Format::format("{G(%2d) ", high);

		clist[j].printed = true;
		output += Format::format("{W%d. %-30s{x%s %s %s %s\n",
		    i + 1,
		    clist[j].clan->clanname,
		    make_bar("t", "C", clist[j].tmembers, clist[j].members, IS_IMMORTAL(ch)),
		    make_bar("h", "G", clist[j].clan->clanqp, clist[j].qp, IS_IMMORTAL(ch)),
		    make_bar("y", "Y", clist[j].clan->gold_balance, clist[j].gold, IS_IMMORTAL(ch)),
		    make_bar("r", "P", clist[j].clan->warcpmod, clist[j].warpts, IS_IMMORTAL(ch)));
	}

	output += "\n";
	page_to_char(output, ch);
}


void print_new_affects(Character *ch)
{
	char torch[8], border[4], breakline[MSL];
	String buffer;
	bool found = false;
	strcpy(border, get_custom_color_code(ch, CSLOT_SCORE_BORDER));
	Format::sprintf(torch, "%s|#|{x", get_custom_color_code(ch, CSLOT_SCORE_TORCH));
	Format::sprintf(breakline, " %s%s----------------------------------------------------------------%s\n", torch, border, torch);

	if (affect::list_char(ch) != nullptr)
		affect::sort_char(ch, affect::comparator_duration);

	// spells
	if (affect::list_char(ch) != nullptr) {
		int affcount = 0;

		for (const affect::Affect *paf = affect::list_char(ch); paf; paf = paf->next)
			if (paf->where == TO_AFFECTS && !paf->permanent)
				affcount++;

		if (affcount > 0) {
			buffer += Format::format(" %s {bYou are affected by the following spells:                      %s\n",
			    torch, torch);
			buffer += breakline;

			const affect::Affect *paf_last = nullptr;
			for (const affect::Affect *paf = affect::list_char(ch); paf != nullptr; paf = paf->next) {
				if (paf->where != TO_AFFECTS || paf->permanent)
					continue;

				String namebuf, modbuf, timebuf;

				if (paf_last != nullptr && paf->type == paf_last->type) {
					if (ch->level >= 20)
						namebuf = "                   ";
					else
						continue;
				}
				else
					Format::sprintf(namebuf, "%-19s", affect::lookup(paf->type).name);

				if (ch->level >= 20) {
					if (paf->location != APPLY_NONE && paf->modifier != 0)
						Format::sprintf(modbuf, "%s by %d",
					        affect_loc_name(paf->location), paf->modifier);

					if (paf->duration != -1)
						Format::sprintf(timebuf, "%3d hrs", paf->duration + 1);
				}

				buffer += Format::format(" %s {b%-19s %s| {b%-30s %s| {b%7s %s\n",
					torch,
					namebuf,
					border,
					modbuf,
					border,
					timebuf,
					torch
				);

				paf_last = paf;
			}

			found = true;
		}
	}

	bool print = false;

	for (int iWear = 0; iWear < MAX_WEAR; iWear++) {
		Object *obj;

		if ((obj = get_eq_char(ch, iWear)) != nullptr) {
			for (const affect::Affect *paf = affect::list_obj(obj); paf; paf = paf->next) {
				if (paf->where != TO_AFFECTS)
					continue;

				if (!print) {
					if (found)
						buffer += breakline;

					buffer += Format::format(" %s {bYou are affected by the following equipment spells:            %s\n",
					    torch, torch);
					buffer += breakline;
					print = true;
					found = true;
				}

				String namebuf, eqbuf, timebuf;

				namebuf = affect::lookup(paf->type).name;
				eqbuf = obj->short_descr.uncolor().substr(0, 38);

				if (paf->duration != -1)
					Format::sprintf(timebuf, "%3d hrs", paf->duration + 1);

				buffer += Format::format(" %s {b%-19s %s| {g%-30s %s| {b%7s %s\n",
					torch,
					namebuf,
					border,
					eqbuf,
					border,
					timebuf,
					torch
				);
			}
		}
	}

	if (affect::list_char(ch) != nullptr) {
		int affcount = 0;

		for (const affect::Affect *paf = affect::list_char(ch); paf; paf = paf->next)
			if (paf->where == TO_AFFECTS && paf->permanent)
				affcount++;

		if (affcount > 0) {
			if (found)
				buffer += breakline;

			buffer += Format::format(" %s {bYou are affected by the following racial abilities:            %s\n",
			    torch, torch);
			buffer += breakline;

			const affect::Affect *paf_last = nullptr;
			for (const affect::Affect *paf = affect::list_char(ch); paf != nullptr; paf = paf->next) {
				if (paf->where != TO_AFFECTS || !paf->permanent)
					continue;

				String namebuf, modbuf;

				if (paf_last != nullptr && paf->type == paf_last->type) {
					if (ch->level >= 20)
						namebuf = "                   ";
					else
						continue;
				}
				else
					Format::sprintf(namebuf, "%-19s", affect::lookup(paf->type).name);

				if (ch->level >= 20) {
					if (paf->location != APPLY_NONE && paf->modifier != 0)
						Format::sprintf(modbuf, "%s by %d",
					        affect_loc_name(paf->location), paf->modifier);
				}

				buffer += Format::format(" %s {b%-19s %s| {b%-40s %s\n",
					torch,
					namebuf,
					border,
					modbuf,
					torch
				);

				paf_last = paf;
			}

			found = true;
		}
	}

	if (!ch->is_npc()
	    && ch->pcdata->remort_count
	    && ch->pcdata->raffect[0]
	    && ch->pcdata->plr_flags.has(PLR_SHOWRAFF)) {

		if (found)
			buffer += breakline;

		buffer += Format::format(" %s {bYou are affected by the following remort affects:              %s\n",
		    torch, torch);
		buffer += breakline;

		for (int raff = 0; raff < ch->pcdata->remort_count / 10 + 1; raff++)
			for (int i = 0; i < MAX_RAFFECTS; i++)
				if (raffects[i].id == ch->pcdata->raffect[raff])
					buffer += Format::format(" %s {b%-62s %s\n",
					    torch, raffects[i].description, torch);

		found = true;
	}

	if (!found)
		buffer += Format::format(" %s {bYou are not affected by any spells.                            %s\n",
		    torch, torch);

	buffer += Format::format(" %s%s================================================================%s\n", torch, border, torch);
	page_to_char(buffer, ch);
}


void score_new(Character *ch)
{
	String buf;
	String border = get_custom_color_code(ch, CSLOT_SCORE_BORDER);
	String torch = get_custom_color_code(ch, CSLOT_SCORE_TORCH);
	String flame = get_custom_color_code(ch, CSLOT_SCORE_FLAME);
//	line  1:   ,                                                                  ,
	ptc(ch, "  %s,                                                                  ,{x\n", flame);
//	line  2:  '`,                Kazander, Lover of Freyja's Soul                '`,
	/* center the name and title */
	new_color(ch, CSLOT_SCORE_TITLE);
	Format::sprintf(buf, "%s%s{x%s", get_custom_color_code(ch, CSLOT_SCORE_NAME), ch->is_npc() ? ch->short_descr : ch->name, ch->is_npc() ? "" : ch->pcdata->title);
	ptc(ch, " %s'`,{x %s %s'`,{x\n", flame, buf.center(62), flame);
//	line  3:  `,                                                                 `,
	ptc(ch, " %s`,                                                                 `,{x\n", flame);
//	line  4:  .:.                        Male Dragon Mage                        .:.
	Format::sprintf(buf, "%s ", sex_table[GET_ATTR_SEX(ch)].name.capitalize());
	buf += race_table[ch->race].name.capitalize();
	buf += " ";
	buf += ch->guild == Guild::none ? "None" : guild_table[ch->guild].name.capitalize();
	new_color(ch, CSLOT_SCORE_CLASS);
	ptc(ch, " {Y.:.{x %s {Y.:.{x\n", buf.center(62));
//	line  5:  )X(           Level 99 (Remort 0)     Age: 17 (130 Hours)          )X(
	ptc(ch, " %s)X({x ", torch);
	new_color(ch, CSLOT_SCORE_LEVEL);
	Format::sprintf(buf, "Level{B:{x %d {B({xRemort %d{B){x     Age{B:{x %d {B({x%d Hours{B){x",
	        ch->level,
	        ch->is_npc() ? 0 : ch->pcdata->remort_count,
	        GET_ATTR_AGE(ch),
	        get_play_hours(ch));
	stc(buf.center(62), ch);
	ptc(ch, " %s)X({x\n", torch);
//	line  6: [%%%]==============================================================[%%%]
	ptc(ch, "%s", torch);
	stc("[%%%]", ch);
	ptc(ch, "%s==============================================================%s", border, torch);
	stc("[%%%]{x\n", ch);
//	line  7:  |#| Strength     25/25 |      Hp 30000/30000  |     Gold     17001 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x     Strength %2d/", torch, GET_ATTR_STR(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ATTR_BASE(ch, APPLY_STR), border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc("      Hp ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->hit, GET_MAX_HIT(ch), border);
	new_color(ch, CSLOT_SCORE_WEALTH);
	ptc(ch, "     Gold %9ld %s|#|{x\n", ch->gold, torch);
//	line  8:  |#| Intelligence 25/25 |    Mana 30000/30000  |   Silver       958 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x Intelligence %2d/", torch, GET_ATTR_INT(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ATTR_BASE(ch, APPLY_INT), border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc("    Mana ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->mana, GET_MAX_MANA(ch), border);
	new_color(ch, CSLOT_SCORE_WEALTH);
	ptc(ch, "   Silver %9ld %s|#|{x\n", ch->silver, torch);
//	line  9:  |#| Wisdom       25/25 | Stamina 30000/30000  |    Items   22/1000 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x       Wisdom %2d/", torch, GET_ATTR_WIS(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ATTR_BASE(ch, APPLY_WIS), border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc(" Stamina ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->stam, GET_MAX_STAM(ch), border);
	new_color(ch, CSLOT_SCORE_ENCUMB);
	ptc(ch, "    Items %4d/%4d %s|#|{x\n", get_carry_number(ch), can_carry_n(ch), torch);
//	line 10:  |#| Dexterity    25/25 |                      |   Weight   53/1000 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x    Dexterity %2d/", torch, GET_ATTR_DEX(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|                      |{x", ATTR_BASE(ch, APPLY_DEX), border);
	new_color(ch, CSLOT_SCORE_ENCUMB);
	ptc(ch, "   Weight %4d/%4d %s|#|{x\n",
	    get_carry_weight(ch),
	    IS_IMMORTAL(ch) ? 9999 : can_carry_w(ch),
	    torch);
//	line 11:  |#| Constitution 25/25 |  Armor vs            |                    |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x Constitution %2d/", torch, GET_ATTR_CON(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ATTR_BASE(ch, APPLY_CON), border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "  Armor vs            %s|                    %s|#|{x\n", border, torch);
//	line 12:  |#| Charisma     25/25 |    Pierce      -600  |  Hitroll       312 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x     Charisma %2d/", torch, GET_ATTR_CHR(ch));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ATTR_BASE(ch, APPLY_CHR), border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "    Pierce    %6d  %s|{x", GET_AC(ch, AC_PIERCE), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("  Hitroll     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", GET_ATTR_HITROLL(ch), torch);
//	line 13:  |#|                    |      Bash      -603  |  Damroll       334 |#|
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, " %s|#|{x                    %s|{x      Bash    %6d  %s|{x",
	    torch, border, GET_AC(ch, AC_BASH), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("  Damroll     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", GET_ATTR_DAMROLL(ch), torch);
//	line 14:  |#| QuestPoints   4012 |     Slash      -596  |    Saves       -23 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x QuestPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->is_npc() ? 0 : ch->pcdata->questpoints, border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "     Slash    %6d  %s|{x", GET_AC(ch, AC_SLASH), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("    Saves     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", GET_ATTR_SAVES(ch), torch);
//	line 15:  |#| SkillPoints      0 |     Magic      -601  |    Wimpy       200 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x SkillPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->is_npc() ? 0 : ch->pcdata->skillpoints, border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "     Magic    %6d  %s|{x", GET_AC(ch, AC_EXOTIC), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("    Wimpy     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", ch->wimpy, torch);
//	line 16:  |#|    RPPoints      0 |                      |                    |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x    RPPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|                      |                    %s|#|{x\n",
	    ch->is_npc() ? 0 : ch->pcdata->rolepoints, border, torch);
//	line 17:  |#|      Trains      4 |  Alignment    -1000  | Experience         |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x      Trains  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->train, border);
	new_color(ch, CSLOT_SCORE_ALIGN);
	ptc(ch, "  Alignment    %s%5d  %s|{x",
	    ch->alignment > 350 ? "{W" : ch->alignment < -350 ? "{R" : "{c", ch->alignment, border);
	new_color(ch, CSLOT_SCORE_XPNAME);
	ptc(ch, " Experience         %s|#|{x\n", torch);
//	line 18:  |#|   Practices     32 |----------------------|    To Level   1000 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x   Practices  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|----------------------|{x", ch->practice, border);
	new_color(ch, CSLOT_SCORE_XPNUM);
	ptc(ch, "    To Level %6ld %s|#|{x\n",
	    ch->is_npc() ? 0 : (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp, torch);
//	line 19:  |#|                    |   You are standing   |       Total  42055 |#|
	Format::sprintf(buf, "You are ");

	switch (get_position(ch)) {
	case POS_DEAD:          buf += "DEAD!!";          break;

	case POS_INCAP:
	case POS_MORTAL:        buf += "dying";           break;

	case POS_STUNNED:       buf += "stunned";         break;

	case POS_SLEEPING:      buf += "sleeping";        break;

	case POS_RESTING:       buf += "resting";         break;

	case POS_STANDING:      buf += "standing";        break;

	case POS_FIGHTING:      buf += "fighting";        break;

	case POS_SITTING:       buf += "sitting";         break;
	case POS_FLYING:        buf += "flying";          break;
	}

	ptc(ch, " %s|#|                    %s|{x", torch, border);
	new_color(ch, CSLOT_SCORE_POSITION);
	ptc(ch, "%s%s|{x", buf.center(22), border);
	new_color(ch, CSLOT_SCORE_XPNUM);
	ptc(ch, "       Total %6d %s|#|{x\n", std::min(ch->exp, 999999), torch);
//	line 20:  |#|----------------------------------------------------------------|#|
	ptc(ch, " %s|#|%s----------------------------------------------------------------%s|#|{x\n",
	    torch, border, torch);
//	line 21:  |#| PK: 000 Wins, 000 Losses  Rank: 0  Arena: 000 Wins, 000 Losses |#|

	if (!ch->is_npc()) {
		new_color(ch, CSLOT_SCORE_PKRECORD);
		Format::sprintf(buf, "PK: %d Wins, %d Losses  %sRank: %d{x  Arena: %d Wins, %d Losses",
		        ch->pcdata->pckills, ch->pcdata->pckilled,
		        get_custom_color_code(ch, CSLOT_SCORE_PKRANK), ch->pcdata->pkrank,
		        ch->pcdata->arenakills, ch->pcdata->arenakilled);
		ptc(ch, " %s|#|{x %s %s|#|{x\n", torch, buf.center(62), torch);
	}
//	line 22:  |#|================================================================|#|
	ptc(ch, " %s|#|%s================================================================%s|#|{x\n",
	    torch, border, torch);

	if (ch->comm_flags.has(COMM_SHOW_AFFECTS))
		print_new_affects(ch);

//	line 23:  |#|                                                                |#|
	ptc(ch, " %s|#|                                                                |#|{x\n", torch);
//	line 24: [[|]]                                                              [[|]]
	ptc(ch, "%s[[|]]                                                              [[|]]{x\n", torch);
}

void do_affects(Character *ch, String argument)
{
	new_color(ch, CSLOT_SCORE_FLAME);
	stc("  ,                                                                  ,\n"
	    " '`,                                                                '`,\n"
	    " `,                                                                 `,\n"
	    " {Y.:.                                                                .:.{x\n", ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc(" )X(                                                                )X(\n"
	    "[%%%]", ch);
	new_color(ch, CSLOT_SCORE_BORDER);
	stc("==============================================================", ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc("[%%%]\n", ch);
	set_color(ch, WHITE, NOBOLD);
	print_new_affects(ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc(" |#|                                                                |#|\n"
	    "[[|]]                                                              [[|]]\n", ch);
	set_color(ch, WHITE, NOBOLD);
}

void do_score(Character *ch, String argument)
{
	score_new(ch);
}

/*
The scry spell lets the caster view the room in which his/her
pet is currently standing in. The room may, however, be
dark.
-- Outsider
*/
void spell_scry(skill::type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *pet;

	/* make sure we have a pet */
	if (! ch->pet) {
		stc("You do not have a pet.\n", ch);
		return;
	}

	pet = ch->pet;

	/* check to see if room is dark */
	if (pet->in_room->is_dark()) {
		stc("The room your pet is in is dark.\n", ch);
		return;
	}
	else {  /* we can see */
		stc("You see...\n  ", ch);
		stc(pet->in_room->description(), ch);
		show_list_to_char(pet->in_room->contents, ch, false, false, false);
		show_char_to_char(pet->in_room->people, ch);
		return;
	}

	return;
}   /* spell_scry end */




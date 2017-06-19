/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#include <vector>

#include "act.hh"
#include "argument.hh"
#include "Affect.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "ExtraDescr.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "String.hh"
#include "tables.hh"

/* Object vnums for object quest 'tokens' */
#define QUEST_OBJQUEST1 1283
#define QUEST_OBJQUEST2 1284
#define QUEST_OBJQUEST3 1285
#define QUEST_OBJQUEST4 1286
#define QUEST_OBJQUEST5 1287

/* syllable tables */
#define MAXMSYL1        77
#define MAXMSYL2        38
#define MAXMSYL3        83
#define MAXFSYL1        76
#define MAXFSYL2        42
#define MAXFSYL3        54

/* Usage info on the QUEST commands -- Elrac */
/* Keep this in line with the do_quest function's keywords */
void quest_usage(Character *ch)
{
	set_color(ch, YELLOW, BOLD);
	stc("QUEST commands: REQUEST INFO TIME COMPLETE FORFEIT POINTS JOIN.\n", ch);

	if (IS_IMMORTAL(ch))
		stc("Imm QUEST commands: OPEN CLOSE LIST DOUBLE DEDUCT PK.\n", ch);

	stc("For more information, type 'HELP QUEST'.\n", ch);
	set_color(ch, WHITE, NOBOLD);
} /* end quest_usage */

void sq_cleanup(Character *ch)
{
	Character *mob;
	Object *obj;
	ch->pcdata->plr_flags -= PLR_SQUESTOR;
	ch->pcdata->squest_giver = 0;
	ch->pcdata->sqcountdown = 0;

	for (obj = object_list; obj != nullptr; obj = obj->next)
		if (obj == ch->pcdata->squestobj)
			extract_obj(obj);

	for (mob = char_list; mob != nullptr ; mob = mob->next)
		if (mob == ch->pcdata->squestmob)
			extract_char(mob, TRUE);

	ch->pcdata->squestobj = nullptr;
	ch->pcdata->squestmob = nullptr;
	ch->pcdata->squestobjf = FALSE;
	ch->pcdata->squestmobf = FALSE;
}

bool quest_level_diff(int clevel, int mlevel)
{
	if (clevel <= 5 && mlevel <= 5)                                    return TRUE;
	else if (clevel >  5 && clevel <= 15 && mlevel >  5 && mlevel <= 15)    return TRUE;
	else if (clevel > 15 && clevel <= 20 && mlevel > 15 && mlevel <= 20)    return TRUE;
	else if (clevel > 20 && clevel <= 25 && mlevel > 20 && mlevel <= 25)    return TRUE;
	else if (clevel > 25 && clevel <= 35 && mlevel > 25 && mlevel <= 35)    return TRUE;
	else if (clevel > 35 && clevel <= 45 && mlevel > 35 && mlevel <= 45)    return TRUE;
	else if (clevel > 45 && clevel <= 50 && mlevel > 45 && mlevel <= 50)    return TRUE;
	else if (clevel > 50 && clevel <= 55 && mlevel > 50 && mlevel <= 55)    return TRUE;
	else if (clevel > 55 && clevel <= 65 && mlevel > 55 && mlevel <= 65)    return TRUE;
	else if (clevel > 65 && clevel <= 75 && mlevel > 65 && mlevel <= 75)    return TRUE;
	else if (clevel > 75 && clevel <= 85 && mlevel > 75 && mlevel <= 85)    return TRUE;
	else if (clevel > 85 && mlevel >  85)                                   return TRUE;
	else                                                                    return FALSE;
}

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */
Character *find_questmaster(Character *ch)
{
	Character *questman;

	for (questman = ch->in_room->people; questman != nullptr; questman = questman->next_in_room) {
		if (!IS_NPC(questman))
			continue;

		if (questman->spec_fun == spec_lookup("spec_questmaster"))
			break;
	}

	if (questman == nullptr)
		return nullptr;

	if (questman->pIndexData == nullptr) {
		Logging::bug("find_questmaster: Questmaster has nullptr pIndexData!", 0);
		questman = nullptr;
	}

	if (questman->fighting != nullptr) {
		stc("Wait until the fighting stops.\n", ch);
		questman = nullptr;
	}

	return questman;
} /* end find_questmaster */

Character *find_squestmaster(Character *ch)
{
	Character *questman;

	for (questman = ch->in_room->people; questman != nullptr; questman = questman->next_in_room) {
		if (!IS_NPC(questman))
			continue;

		if (questman->spec_fun == spec_lookup("spec_squestmaster"))
			break;
	}

	if (questman == nullptr)
		return nullptr;

	if (questman->pIndexData == nullptr) {
		Logging::bug("find_questmaster: Questmaster has nullptr pIndexData!", 0);
		questman = nullptr;
	}

	if (questman->fighting != nullptr) {
		stc("Wait until the fighting stops.\n", ch);
		questman = nullptr;
	}

	return questman;
} /* end find_squestmaster */

/* Obtain additional location information about sought item/mob */
void quest_where(Character *ch, char *what)
{
	RoomPrototype *room;

	if (ch->questloc <= 0) {
		Logging::bug("QUEST INFO: ch->questloc = %d", ch->questloc);
		return;
	}

	if (ch->in_room == nullptr)
		return;

	room = get_room_index(ch->questloc);

	if (room->area == nullptr) {
		Logging::bug("QUEST INFO: room(%d)->area == nullptr", ch->questloc);
		return;
	}

	ptc(ch, "Rumor has it this %s was last seen in the area known as %s", what, room->area->name);

	if (room->name.empty())
		ptc(ch, ".\n");
	else
		ptc(ch, ",\nnear %s.\n", room->name);
} /* end quest_where */

void squest_info(Character *ch)
{
	MobilePrototype *questman;
	RoomPrototype *questroom_obj, *questroom_mob;

	if (!ch->pcdata->plr_flags.has(PLR_SQUESTOR)) {
		stc("You aren't currently on a skill quest.\n", ch);
		return;
	}

	if (ch->pcdata->squest_giver < 1) {
		Logging::bug("QUEST INFO: quest giver = %d", ch->pcdata->squest_giver);
		stc("It seems the questmistress died of old age waiting for you.\n", ch);
		ch->pcdata->plr_flags -= PLR_SQUESTOR;
		return;
	}

	questman = get_mob_index(ch->pcdata->squest_giver);

	if (questman == nullptr) {
		Logging::bug("QUEST INFO: skill quest giver %d has no MobilePrototype!", ch->quest_giver);
		stc("The questmistress has fallen very ill. Please contact an imm!\n", ch);
		ch->pcdata->plr_flags -= PLR_SQUESTOR;
		return;
	}

	if (ch->pcdata->squestobj == nullptr && ch->pcdata->squestmob == nullptr) { /* no quest */
		stc("You've forgotten what your skill quest was.\n", ch);
		Logging::bug("QUEST INFO: skill quest with no obj or mob", 0);
		sq_cleanup(ch);
		return;
	}

	ptc(ch, "You recall the skill quest which %s gave you.\n", questman->short_descr);

	if (ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob == nullptr) { /* obj, no mob */
		if (ch->pcdata->squestobjf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr.empty() ? "your quest master" : questman->short_descr));
			return;
		}

		if ((questroom_obj = get_room_index(ch->pcdata->squestloc1)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj quest with no location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_obj->area == nullptr) {
			Logging::bug("QUEST INFO: sqobj location with no area", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s!\n",
		    ch->pcdata->squestobj->short_descr);
		ptc(ch, "The artifact was last known to be in %s{x,\n", questroom_obj->name);
		ptc(ch, "in the area known as %s{x.\n", questroom_obj->area->name);
		return;
	}

	if (ch->pcdata->squestobj == nullptr && ch->pcdata->squestmob != nullptr) { /* mob, no obj */
		if (ch->pcdata->squestmobf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr.empty() ? "the questmistress" : questman->short_descr));
			return;
		}

		if ((questroom_mob = get_room_index(ch->pcdata->squestloc2)) == nullptr) {
			Logging::bug("QUEST INFO: sqmob quest with no location", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_mob->area == nullptr) {
			Logging::bug("QUEST INFO: sqmob location with no area", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		ptc(ch, "You are on a quest to learn from the legendary %s!\n",
		    ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name);
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
		return;
	}

	if (ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob != nullptr) { /* mob and obj */
		if ((questroom_obj = get_room_index(ch->pcdata->squestloc1)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj/mob quest with no obj location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_obj->area == nullptr) {
			Logging::bug("QUEST INFO: sqobj location with no area", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if ((questroom_mob = get_room_index(ch->pcdata->squestloc2)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj/mob quest with no mob location", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_mob->area == nullptr) {
			Logging::bug("QUEST INFO: sqmob location with no area", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (ch->pcdata->squestobjf && ch->pcdata->squestmobf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr.empty() ? "the questmistress" : questman->short_descr));
			return;
		}

		if (!ch->pcdata->squestmobf && ch->pcdata->squestobjf) {
			ptc(ch, "You must return the %s to %s.\n",
			    ch->pcdata->squestobj->short_descr, ch->pcdata->squestmob->short_descr);
			ptc(ch, "%s can usually be found in %s{x,\n",
			    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name);
			ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s,\n", ch->pcdata->squestobj->short_descr);
		ptc(ch, "from %s{x, in the area known as %s{x,\n", questroom_obj->name, questroom_obj->area->name);
		ptc(ch, "and return it to it's rightful owner, %s.\n", ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name);
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
	}
}

void quest_info(Character *ch)
{
	MobilePrototype *questman, *questinfo;
	ObjectPrototype *questinfoobj;

	if (!ch->act_flags.has(PLR_QUESTOR)) {
		stc("You aren't currently on a quest.\n", ch);
		return;
	}

	if (ch->quest_giver < 1) {
		Logging::bug("QUEST INFO: quest giver = %d", ch->quest_giver);
		stc("It seems the questmaster died of old age waiting for you.\n", ch);
		ch->act_flags -= PLR_QUESTOR;
		return;
	}

	questman = get_mob_index(ch->quest_giver);

	if (questman == nullptr) {
		Logging::bug("QUEST INFO: quest giver %d has no MobilePrototype!", ch->quest_giver);
		stc("The questmaster has fallen very ill. Please contact an imm!\n", ch);
		ch->act_flags -= PLR_QUESTOR;
		return;
	}

	if (ch->questmob == -1) { /* killed target mob */
		stc("Your quest is ALMOST complete!\n", ch);
		ptc(ch, "Get back to %s before your time runs out!\n",
		    (questman->short_descr.empty() ? "your quest master" : questman->short_descr));
		return;
	}
	else if (ch->questobj > 0) { /* questing for an object */
		questinfoobj = get_obj_index(ch->questobj);

		if (questinfoobj != nullptr) {
			stc("You recall the quest which the questmaster gave you.\n", ch);
			ptc(ch, "You are on a quest to recover the fabled %s!\n", questinfoobj->name);
			quest_where(ch, "treasure");
			return;
		}

		/* quest object not found! */
		Logging::bug("No info for quest object %d", ch->questobj);
		ch->questobj = 0;
		ch->questobf = 0;
		ch->act_flags -= PLR_QUESTOR;
		/* no RETURN -- fall thru to 'no quest', below */
	}
	else if (ch->questmob > 0) { /* questing for a mob */
		questinfo = get_mob_index(ch->questmob);

		if (questinfo != nullptr) {
			ptc(ch, "You are on a quest to slay the dreaded %s!\n", questinfo->short_descr);
			quest_where(ch, "fiend");
			return;
		}

		/* quest mob not found! */
		Logging::bug("No info for quest mob %d", ch->questmob);
		ch->questmob = 0;
		ch->act_flags -= PLR_QUESTOR;
		/* no RETURN -- fall thru to 'no quest', below */
	}

	/* we shouldn't be here */
	Logging::bug("QUEST INFO: Questor with no kill, mob or obj", 0);
	return;
}

int get_random_skill(Character *ch)
{
	int count;
	int sn = 0;
	int pass = 1;
	int target = 0;

	/* pick a random skill or spell, if they have any */
	while (pass != 0) {
		count = 0;

		for (sn = 0; sn < skill_table.size(); sn++) {
			if (ch->pcdata->learned[sn] <= 0
			    || ch->pcdata->learned[sn] >= 100)
				continue;

			if (skill_table[sn].remort_class > 0 && !CAN_USE_RSKILL(ch, sn))
				continue;

			if (skill_table[sn].skill_level[ch->cls] > ch->level)
				continue;

			if (pass == 2 && count == target) {
				pass = 0;               /* get out of while loop */
				break;                  /* get out of for loop */
			}

			count++;
		}

		if (count == 0) { /* no skills or spells found */
			pass = 0;
			sn = -1;
		}

		if (pass != 0) {
			target = number_range(0, count);
			pass = 2;
		}
	}

	return sn;
}

void squestobj_to_squestmob(Character *ch, Object *obj, Character *mob)
{
	char buf[MAX_STRING_LENGTH];
	check_social(mob, "beam", ch->name.c_str());
	Format::sprintf(buf, "Thank you, %s, for returning my lost %s!", ch->name, obj->short_descr);
	do_say(mob, buf);
	extract_obj(obj);
	do_say(mob, "I left the gold reward with the Questmistress.  Farewell, and thank you!");
	act("$n turns and walks away.", mob, nullptr, nullptr, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has returned %s to %s", obj->short_descr, mob->short_descr);
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
	extract_char(mob, TRUE);
	ch->pcdata->squestmobf = TRUE;
}

void squestmob_found(Character *ch, Character *mob)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->pcdata->squestmobf) {
		Logging::bug("At squestmob_found, player's squestmob already found.  Continuing...", 0);
		ch->pcdata->squestmobf = FALSE;
	}

	if (mob->act_flags.has(ACT_MAGE)) {
		switch (number_range(1, 1)) {
		case 1:
			do_say(mob, "Greetings, young spellcaster!");
			do_say(mob, "I will teach you about magic in it's most basic form.");
			do_say(mob, "It is important to concentrate on clearly envisioning\n"
			       "the forming spell in your mind.  Simple spells require less\n"
			       "thought, and have more room for error, but focusing on the\n"
			       "spell, shaping the magic in your mind, perfecting it, is what\n"
			       "sets apart the spellcaster from the wizard.");
			act("$N leads you through some simple exercises to clear your mind,", ch, nullptr, mob, TO_CHAR);
			act("then helps you to visualize the abstract logic required to form", ch, nullptr, mob, TO_CHAR);
			act("a fireball.", ch, nullptr, mob, TO_CHAR);
			act("$N touches a finger to $n's head, and $e looks more relaxed.", ch, nullptr, mob, TO_NOTVICT);
			do_say(mob, "Any mage can cast a simple fireball.  But a true student of\n"
			       "magic will look for new ways, and cast a more powerful spell in the end.");
			act("In your mind, the fireball starts to glow brighter, it's color", ch, nullptr, mob, TO_CHAR);
			act("turning to a soft green.", ch, nullptr, mob, TO_CHAR);
			act("Beads of sweat start to run down $n's forehead as $e concentrates.", ch, nullptr, mob, TO_NOTVICT);
			do_say(mob, "The truly great magicians strive to make spells their own.");
			act("The fireball in your mind fades.", ch, nullptr, mob, TO_CHAR);
			act("$n opens $s eyes.", ch, nullptr, mob, TO_NOTVICT);
			break;
		}
	}
	else if (mob->act_flags.has(ACT_CLERIC)) {
		switch (number_range(1, 1)) {
		case 1:
			act("$N smiles warmly at $n.", ch, nullptr, mob, TO_NOTVICT);
			act("$N smiles warmly at you.", ch, nullptr, mob, TO_CHAR);
			do_say(mob, "Persons of a godly profession act in their deity's name.");
			do_say(mob, "One cannot always know their deity's true thoughts and\n"
			       "intent, so you must use your best judgement.");
			do_say(mob, "Will your deity be pleased with your actions?  You can\n"
			       "never know for certain, but you will quickly know if not.\n");
			do_say(mob, "But, whilst you act in good faith, you will never be\n"
			       "totally alone in your decisions.");
			break;
		}
	}
	else if (mob->act_flags.has(ACT_THIEF)) {
		switch (number_range(1, 1)) {
		case 1:
			ptc(ch, "You feel a tap on your shoulder, and turn to see %s.\n", mob->short_descr);
			do_say(mob, "Many people express distaste at the life of a lowly pickpocket.");
			do_say(mob, "But, few occupations require such skill, dexterity, and daring.");
			act("$n starts into a long monologue detailing the ethics of theft for a living.", mob, nullptr, nullptr, TO_ROOM);
			do_say(mob, "Is it right to steal?  I'll let you decide for yourself, no one\n"
			       "  needs another to explain moral values.  But, for me...");
			act("$N hands $n $s gold pouch, and winks.", ch, nullptr, mob, TO_NOTVICT);
			act("$N hands you your gold pouch, and winks.", ch, nullptr, mob, TO_CHAR);
			break;
		}
	}
	else {  /* all sqmobs should have one flag, revert to warrior just in case */
		switch (number_range(1, 2)) {
		case 1:
			act("$n draws $s blade and places it over $s shoulder.", mob, nullptr, nullptr, TO_ROOM);
			do_say(mob, "The blade is of the most powerful of weapons.");
			do_say(mob, "It can be used for close combat, distanced, and defense.");
			act("$n takes a fighting stance, raising $s blade into the air.", mob, nullptr, nullptr, TO_ROOM);
			do_say(mob, "In one fluid movement one can overcome an untrained oppenent.");
			act("$n whirls the blade down swiftly, pulls it back up into\n"
			    " the air and slices down again.", mob, nullptr, nullptr, TO_ROOM);
			do_say(mob, "A blade can be used for good, or for evil.");
			act("$n pulls the blade back up and slides it into its sheath.", mob, nullptr, nullptr, TO_ROOM);
			do_say(mob, "But its not the blade who decides...");
			break;

		case 2:
			act("$N draws $S blade and motions toward $n.", ch, nullptr, mob, TO_NOTVICT);
			act("$N draws $S blade and motions toward you.", ch, nullptr, mob, TO_CHAR);
			do_say(mob, "A blade is true, unyeilding, and unstoppable.");
			do_say(mob, "It can be used to protect, to defend, to destroy, to disarm.");
			act("$N positions $S blade to point toward $n.", ch, nullptr, mob, TO_NOTVICT);
			act("$N positions $S blade to point toward you.", ch, nullptr, mob, TO_CHAR);
			do_say(mob, "The soul of a blade is within you, and only by your hand can it be awakened.");
			break;
		}
	}

	act("$n turns and walks away.", mob, nullptr, nullptr, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has spoken with %s", mob->short_descr);
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
	extract_char(mob, TRUE);
	ch->pcdata->squestmobf = TRUE;
}

Object *generate_skillquest_obj(Character *ch, int level)
{
	ExtraDescr *ed;
	Object *questobj;
	char buf[MAX_STRING_LENGTH];
	int num_objs, descnum;
	struct sq_item_type {
		char *name;
		char *shortdesc;
		char *longdesc;
		char *extended;
	};
	const struct sq_item_type sq_item_table [] = {
		{
			"quest spellbook lotus",       "Spellbook of Lotus",
			"The Spellbook of Lotus is lying here, waiting for a lucky finder.",
			"A large, heavy book, worn with use. A tiny scrap of paper marks where the last reader's attention was.\n"
		},

		{
			"quest sword furey",           "Sword of Furey",
			"The Sword of Furey is lying here, waiting for a lucky finder.",
			"This large blade's edges, even as stained as they are, seem to glisten with an inner strength.\n"
		},

		{
			"quest lockpicks kahn",        "Lockpicks of Kahn",
			"The Lockpicks of Kahn are lying here, waiting for a lucky finder.",
			"A small grey pouch of supple leather holds the lockpicks of the legendary thief.\n"
		},

		{
			"quest robe alander",          "Robe of Alander",
			"The Robe of Alander is lying here, waiting for a lucky finder.",
			"Though quite dusty, this soft robe looks well used and gives of a warm, comforting glow.\n"
		},

		{nullptr, nullptr, nullptr, nullptr}
	};

	/* count the objects */
	for (num_objs = 0; sq_item_table[num_objs].name != nullptr; num_objs++);

	num_objs--;
	descnum = number_range(0, num_objs);
	questobj = create_object(get_obj_index(OBJ_VNUM_SQUESTOBJ), level);

	if (! questobj) {
		Logging::bug("Memory error creating quest object.", 0);
		return nullptr;
	}

	questobj->timer = (4 * ch->pcdata->sqcountdown + 10) / 3;
	Format::sprintf(buf, "%s %s", sq_item_table[descnum].name, ch->name);
	questobj->name        = buf;
	questobj->short_descr = sq_item_table[descnum].shortdesc;
	questobj->description = sq_item_table[descnum].longdesc;
	ed = new_extra_descr();
	ed->keyword        = sq_item_table[descnum].name;
	ed->description    = sq_item_table[descnum].extended;
	ed->next           = questobj->extra_descr;
	questobj->extra_descr   = ed;
	ch->pcdata->squestobj = questobj;
	return questobj;
}

RoomPrototype *generate_skillquest_room(Character *ch, int level)
{
	RoomPrototype *room, *prev;

	for (; ;) {
		room = get_room_index(number_range(0, 32767));

		if (room == nullptr
		    || !can_see_room(ch, room)
		    || room->area == Game::world().quest.area
		    || room->area->low_range > level
		    || room->area->high_range < level
		    || (room->area->min_vnum >= 24000      /* clanhall vnum ranges */
		        && room->area->min_vnum <= 26999)
		    || room->guild
		    || room->area->name == "Playpen"
		    || room->area->name == "IMM-Zone"
		    || room->area->name == "Limbo"
		    || room->area->name == "Midgaard"
		    || room->area->name == "New Thalos"
		    || room->area->name == "Eilyndrae"     /* hack to make eilyndrae and torayna cri unquestable */
		    || room->area->name == "Torayna Cri"
		    || room->area->name == "Battle Arenas"
		    || room->sector_type == SECT_ARENA
		    || GET_ROOM_FLAGS(room).has_any_of(
		              ROOM_MALE_ONLY
		              | ROOM_FEMALE_ONLY
		              | ROOM_PRIVATE
		              | ROOM_SOLITARY
		              | ROOM_NOQUEST))
			continue;

		/* no pet shops */
		if ((prev = get_room_index(room->vnum - 1)) != nullptr)
			if (GET_ROOM_FLAGS(prev).has(ROOM_PET_SHOP))
				continue;

		return room;
	}
}

void generate_skillquest_mob(Character *ch, Character *questman, int level, int type)
{
	Object  *questobj;
	Character *questmob;
	RoomPrototype *questroom;
	int x, maxnoun;
	char buf[MAX_STRING_LENGTH];
	char longdesc[MAX_STRING_LENGTH];
	char shortdesc[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	String title;
	char *quest;
	title = quest = "";             /* ew :( */

	if ((questmob = create_mobile(get_mob_index(MOB_VNUM_SQUESTMOB))) == nullptr) {
		Logging::bug("Bad skillquest mob vnum, from generate_skillquest_mob", 0);
		return;
	}

	ATTR_BASE(ch, APPLY_SEX) = number_range(1, 2);
	questmob->level = ch->level;

	/* generate name */
	if (GET_ATTR_SEX(questmob) == 1)
		Format::sprintf(shortdesc, "%s%s%s",
		        Msyl1[number_range(0, (MAXMSYL1 - 1))],
		        Msyl2[number_range(0, (MAXMSYL2 - 1))],
		        Msyl3[number_range(0, (MAXMSYL3 - 1))]);
	else
		Format::sprintf(shortdesc, "%s%s%s",
		        Fsyl1[number_range(0, (MAXFSYL1 - 1))],
		        Fsyl2[number_range(0, (MAXFSYL2 - 1))],
		        Fsyl3[number_range(0, (MAXFSYL3 - 1))]);

	Format::sprintf(name, "squestmob %s", shortdesc);
	questmob->name = name;
	questmob->short_descr = shortdesc;

	/* generate title */
	switch (number_range(1, 4)) {
	case 1:
		for (maxnoun = 0; maxnoun < MagT_table.size(); maxnoun++);

		x = number_range(0, --maxnoun);
		title = GET_ATTR_SEX(questmob) == 1 ? MagT_table[x].male : MagT_table[x].female;
		quest = "the powers of magic";
		questmob->act_flags += ACT_MAGE;
		break;

	case 2:
		for (maxnoun = 0; maxnoun < CleT_table.size(); maxnoun++);

		x = number_range(0, --maxnoun);
		title = GET_ATTR_SEX(questmob) == 1 ? CleT_table[x].male : CleT_table[x].female;
		quest = "the wisdom of holiness";
		questmob->act_flags += ACT_CLERIC;
		break;

	case 3:
		for (maxnoun = 0; maxnoun < ThiT_table.size(); maxnoun++);

		x = number_range(0, --maxnoun);
		title = GET_ATTR_SEX(questmob) == 1 ? ThiT_table[x].male : ThiT_table[x].female;
		quest = "the art of thievery";
		questmob->act_flags += ACT_THIEF;
		break;

	case 4:
		for (maxnoun = 0; maxnoun < WarT_table.size(); maxnoun++);

		x = number_range(0, --maxnoun);
		title = GET_ATTR_SEX(questmob) == 1 ? WarT_table[x].male : WarT_table[x].female;
		quest = "the ways of weaponcraft";
		questmob->act_flags += ACT_WARRIOR;
		break;
	}

	Format::sprintf(longdesc, "The %s, %s, stands here.\n", title, questmob->short_descr);
	questmob->long_descr = longdesc;

	if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
		Logging::bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
		return;
	}

	char_to_room(questmob, questroom);
	ch->pcdata->squestmob = questmob;

	if (type == 1) { /* mob quest */
		Format::sprintf(buf, "Seek out the %s, %s, for teachings in %s!", title, questmob->short_descr, quest);
		do_say(questman, buf);
		Format::sprintf(buf, "%s resides somewhere in the area of %s{x,", questmob->short_descr, questmob->in_room->area->name);
		do_say(questman, buf);
		Format::sprintf(buf, "and can usually be found in %s{x.", questmob->in_room->name);
		do_say(questman, buf);
	}
	else if (type == 2) {   /* obj to mob quest */
		if ((questobj = generate_skillquest_obj(ch, level)) == nullptr) {
			Logging::bug(" Bad generate_skillquest_obj, from generate_skillquest_mob", 0);
			return;
		}

		for (; ;) {
			if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
				Logging::bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
				return;
			}

			if (questroom->area == questmob->in_room->area)
				continue;

			obj_to_room(questobj, questroom);
			break;
		}

		Format::sprintf(buf, "The %s %s{x has reported missing the legendary artifact,",
		        title, questmob->short_descr);
		do_say(questman, buf);
		Format::sprintf(buf, "the %s{x!  %s{x resides somewhere in %s{x,",
		        questobj->short_descr, questmob->short_descr, questmob->in_room->area->name);
		do_say(questman, buf);
		Format::sprintf(buf, "and can usually be found in %s{x.", questmob->in_room->name);
		do_say(questman, buf);
		Format::sprintf(buf, "%s last recalls travelling through %s{x, in the",
		        GET_ATTR_SEX(questmob) == 1 ? "He" : "She", questobj->in_room->name);
		do_say(questman, buf);
		Format::sprintf(buf, "area of %s{x, when %s lost the treasure.",
		        questobj->in_room->area->name, GET_ATTR_SEX(questmob) == 1 ? "he" : "she");
		do_say(questman, buf);
	}
}

void generate_skillquest(Character *ch, Character *questman)
{
	Object *questobj;
	RoomPrototype *questroom;
	char buf[MAX_STRING_LENGTH];

	int level = URANGE(1, ch->level, LEVEL_HERO);
	ch->pcdata->sqcountdown = number_range(15, 30);
	ch->pcdata->squest_giver = questman->pIndexData->vnum;

	/* 40% chance of an item quest */
	if (chance(40)) {
		if ((questobj = generate_skillquest_obj(ch, level)) == nullptr) {
			Logging::bug("Bad generate_skillquest_obj, from generate_skillquest", 0);
			return;
		}

		if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
			Logging::bug("Bad generate_skillquest_room, from generate_skillquest", 0);
			return;
		}

		obj_to_room(questobj, questroom);
		ch->pcdata->squestloc1 = questroom->vnum;
		ch->pcdata->squestobj = questobj;
		Format::sprintf(buf, "The legendary artifact, the %s{x, has been reported stolen!", questobj->short_descr);
		do_say(questman, buf);
		Format::sprintf(buf, "The thieves were seen heading in the direction of %s{x,", questroom->area->name);
		do_say(questman, buf);
		do_say(questman, "and witnesses in that area say that they travelled");
		Format::sprintf(buf, "through %s{x.", questroom->name);
		do_say(questman, buf);
		return;
	}
	/* 40% chance of a mob quest */
	else if (chance(66)) {
		generate_skillquest_mob(ch, questman, level, 1);
		ch->pcdata->squestloc2 = ch->pcdata->squestmob->in_room->vnum;
	}
	/* 20% chance of a obj to mob quest */
	else {
		generate_skillquest_mob(ch, questman, level, 2);
		ch->pcdata->squestloc1 = ch->pcdata->squestobj->in_room->vnum;
		ch->pcdata->squestloc2 = ch->pcdata->squestmob->in_room->vnum;
	}
}

void generate_quest(Character *ch, Character *questman)
{
	Character *victim;
	RoomPrototype *room;
	Object *questitem;
	char buf [MAX_STRING_LENGTH];
	/*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */
	/*  Here is the new, improved, align-respecting version of the quest
	victim selection process. It requires at least twice as much CPU
	time as the previous version, so I hope the results are worth it! */
	int aligned;
	int total;
	int pass;
	int target = 0;
	/*  The following outer while(1) loop is needed to go through the mob
	selection twice: Once to count the total number of candidate mobs
	(fulfilling level and other criteria) and the number of suitably
	aligned candidate mobs; and the second (indicated by either pass 2
	or pass 3) to find a randomly chosen n-th aligned candidate or
	candidate. */
	pass = 1;

	while (pass != 0) {
		aligned = 0;
		total = 0;

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (!IS_NPC(victim)
			    || victim->pIndexData == nullptr
			    || victim->in_room == nullptr
			    || victim->pIndexData->pShop != nullptr
			    || victim->act_flags.has(ACT_NOSUMMON)
			    || victim->act_flags.has(ACT_PET)
			    || !strcmp(victim->in_room->area->name, "Playpen")
			    || victim->in_room->clan
			    || affect_exists_on_char(victim, gsn_charm_person)
			    || GET_ROOM_FLAGS(victim->in_room).has_any_of(ROOM_PRIVATE | ROOM_SOLITARY)
			    || GET_ROOM_FLAGS(victim->in_room).has_any_of(ROOM_SAFE | ROOM_MALE_ONLY | ROOM_FEMALE_ONLY)
			    || quest_level_diff(ch->level, victim->level) != TRUE)
				continue;

			/* All conditions met. Increment counters. */
			total++;

			if (IS_NEUTRAL(ch)
			    || (IS_GOOD(ch) && !IS_GOOD(victim))
			    || (IS_EVIL(ch) && !IS_EVIL(victim)))
				aligned++;

			if ((pass == 2 && aligned == target)
			    || (pass == 3 && total == target)) {
				pass = 0; /* stop the outer WHILE */
				break;    /* get out of FOR loop */
			}
		} /* end for victim */

		/* Evaluate the results of first scan through the potential victims. */
		if (pass == 1) {
			if (aligned > 0) {
				target = number_range(1, aligned);
				pass = 2;
			}
			else if (total > 0) {
				target = number_range(1, total);
				pass = 3;
			}
			else {
				/* no victims found */
				victim = nullptr;
				break; /* BREAK for no victims found */
			}
		}
	} /* end while */

	if (victim == nullptr) {
		do_say(questman, "I'm sorry, but I don't have any quests for you at this time.");
		do_say(questman, "Try again later.");
		ch->nextquest = 1;
		return;
	}

	/* at this point the player is sure to get a quest */
	room = victim->in_room;
	ch->quest_giver = questman->pIndexData->vnum;
	ch->questloc = room->vnum;

	/*  40% chance it will send the player on a 'recover item' quest. */
	if (chance(40)) {
		int objvnum = 0;

		switch (number_range(0, 4)) {
		case 0: objvnum = QUEST_OBJQUEST1;      break;

		case 1: objvnum = QUEST_OBJQUEST2;      break;

		case 2: objvnum = QUEST_OBJQUEST3;      break;

		case 3: objvnum = QUEST_OBJQUEST4;      break;

		case 4: objvnum = QUEST_OBJQUEST5;      break;
		}

		questitem = create_object(get_obj_index(objvnum), ch->level);

		if (! questitem) {
			Logging::bug("Error creating quest item.", 0);
			return;
		}

		/* Add player's name to mox name to prevent visibility by others */
		Format::sprintf(buf, "%s %s", questitem->name, ch->name);
		questitem->name = buf;
		/* Mox timer added by Demonfire as a preventative measure against cheating.
		The countdown timer assignment was moved here so that it could be used
		in the mox timer calculation, it was normally assigned after the return
		of this function. */
		ch->countdown = number_range(10, 30);
		questitem->timer = (4 * ch->countdown + 10) / 3;
		obj_to_room(questitem, room);
		ch->questobj = questitem->pIndexData->vnum;
		Format::sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr);
		do_say(questman, buf);
		do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
		Format::sprintf(buf, "Look in the general area of %s for %s!", room->area->name, room->name);
		do_say(questman, buf);
		return;
	}
	else {
		/* Quest to kill a mob */
		switch (number_range(0, 3)) {
		case 0:
			Format::sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",
			        victim->short_descr);
			do_say(questman, buf);
			do_say(questman, "This threat must be eliminated!");
			break;

		case 1:
			Format::sprintf(buf, "Thera's most heinous criminal, %s, has escaped from the dungeon!",
			        victim->short_descr);
			do_say(questman, buf);
			Format::sprintf(buf, "Since the escape, %s has murdered %d civilians!",
			        victim->short_descr, number_range(2, 20));
			do_say(questman, buf);
			do_say(questman, "The penalty for this crime is death, and you are to deliver the sentence!");
			break;

		case 2:
			Format::sprintf(buf, "The Mayor of Midgaard has recently been attacked by %s.  This is an act of war!",
			        victim->short_descr);
			do_say(questman, buf);
			Format::sprintf(buf, "%s must be severly dealt with for this injustice.",
			        victim->short_descr);
			do_say(questman, buf);
			break;

		case 3:
			Format::sprintf(buf, "%s has been stealing valuables from the citizens of Solace.",
			        victim->short_descr);
			do_say(questman, buf);
			Format::sprintf(buf, "Make sure that %s never has the chance to steal again.",
			        victim->short_descr);
			do_say(questman, buf);
			break;
		}

		if (!room->name.empty()) {
			Format::sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",
			        victim->short_descr, room->name);
			do_say(questman, buf);
			Format::sprintf(buf, "That location is in the general area of %s.", room->area->name);
			do_say(questman, buf);
		}

		ch->countdown = number_range(10, 30);
		ch->questmob = victim->pIndexData->vnum;
	}
} /* end generate_quest() */

/* The main quest function */
void do_quest(Character *ch, String argument)
{
	Character *questman;
	char buf [MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		do_say(ch, "Don't be silly, mobs can't quest!");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty()) {
		quest_usage(ch);
		return;
	}

	/* The following subfunctions are grouped in alphabetical order so I can find them.
	   Each is an independent IF block. Imm commands come after mortal commands starting
	   with the same letter so that imms do not execute the more powerful commands by
	   accident when they abbreviate too much. */

	/*** AWARD ***/
	if (IS_IMP(ch) && arg1.is_prefix_of("award")) {
		int number = -1;
		Character *wch;
		Descriptor *d;

		String player;
		argument = one_argument(argument, player);

		if (player[0] == '\0') {
			stc("Quest award to whom?\n", ch);
			return;
		}

		wch = get_player_world(ch, player, VIS_PLR);

		if (wch == nullptr && !player.has_prefix("allchars")) {
			ptc(ch, "You find no player named '%s' in the game!\n", player);
			return;
		}

		if (!argument.empty())
			number = atoi(argument);

		if (number < 0 || number > 10) {
			ptc(ch, "Award %s how many quest points (0..10) ?\n", wch->name);
			return;
		}

		if (player.has_prefix("allchars")) {
			for (d = descriptor_list; d; d = d->next) {
				if (IS_PLAYING(d) && !IS_IMMORTAL(d->character)) {
					d->character->questpoints += number;
					ptc(ch, "You award %s %d questpoints.\n", d->character->name, number);
					ptc(d->character, "%s has awarded you %d questpoints.\n", ch->name, number);
				}
			}

			Format::sprintf(buf, "Log %s: QUEST AWARD allchars %d", ch->name, number);
			wiznet(buf, ch, nullptr, WIZ_SECURE, 0, GET_RANK(ch));
			Logging::log(buf);
			return;
		}

		wch->questpoints += number;
		ptc(ch, "You award %d quest points to %s.\n", number, wch->name);
		ptc(wch, "%s awards %d quest points to you.\n", ch->name, number);
		Format::sprintf(buf, "Log %s: QUEST AWARD %d to %s", ch->name, number, player);
		wiznet(buf, ch, nullptr, WIZ_SECURE, 0, GET_RANK(ch));
		Logging::log(buf);
		return;
	}

	/*** COMPLETE ***/
	if (arg1.is_prefix_of("complete")) {
		Object *obj = nullptr;
		int pointreward = 0;
		int reward = 0;
		int pracreward = 0;
		int sn = 0;

		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if ((questman = find_squestmaster(ch)) != nullptr) {
			act("$n informs $N $e has completed $s skill quest.", ch, nullptr, questman, TO_ROOM);
			act("You inform $N you have completed your skill quest.", ch, nullptr, questman, TO_CHAR);

			if (ch->pcdata->squest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a skill quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!ch->pcdata->plr_flags.has(PLR_SQUESTOR)) {
				do_say(questman, "But you aren't on a skill quest at the moment!");
				Format::sprintf(buf, "You have to REQUEST a skill quest first, %s.", ch->name);
				do_say(questman, buf);
				/* clean up, just in case */
				sq_cleanup(ch);
				return;
			}

			if (ch->pcdata->nextsquest > 0 || ch->pcdata->sqcountdown <= 0) {
				do_say(questman, "But you didn't complete your skill quest in time!");
				/* clean up, just in case */
				sq_cleanup(ch);
				return;
			}

			/* if they're looking for an object, and not a mob */
			if (ch->pcdata->squestobj && !ch->pcdata->squestmob) {
				for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
					if (obj->pIndexData == nullptr) {
						Logging::bug("SQUEST COMPLETE: obj with nullptr pIndexData", 0);
						obj = nullptr;
						break;
					}
					else if (obj == ch->pcdata->squestobj)
						break;
				}

				/* check to see if they dropped it on the way */
				if (ch->pcdata->squestobjf && obj == nullptr) {
					do_say(questman, "You must have lost the item on the way here.  Hurry and find it!");
					ch->pcdata->squestobjf = FALSE;
					return;
				}

				/* if they haven't found it */
				if (!ch->pcdata->squestobjf) {
					do_say(questman, "You haven't found the artifact yet, but there is still time!");
					return;
				}

				act("You hand the $p to $N.", ch, obj, questman, TO_CHAR);
				act("$n hands the $p to $N.", ch, obj, questman, TO_ROOM);
				extract_obj(obj);
			}
			/* if they're looking for a mob, and not an item */
			else if (ch->pcdata->squestmob && !ch->pcdata->squestobj) {
				if (!ch->pcdata->squestmobf) {
					Format::sprintf(buf, "You haven't spoken with %s yet!  Come back when you have.",
					        ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}
			}
			/* if they're looking for both a mob and an object */
			else if (ch->pcdata->squestmob && ch->pcdata->squestobj) {
				if (ch->pcdata->squestobjf && !ch->pcdata->squestmobf) {
					Format::sprintf(buf, "You must return the %s to %s before you return here.",
					        ch->pcdata->squestobj->short_descr, ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}

				if (ch->pcdata->squestmobf && !ch->pcdata->squestobjf) { /* shouldn't happen */
					Logging::bug("SQuest mob found without object", 0);
					do_say(questman, "%s must like you a great deal...");
					sq_cleanup(ch);
					return;
				}

				if (!ch->pcdata->squestmobf && !ch->pcdata->squestobjf) {
					do_say(questman, "You haven't completed your quest yet, but there is still time!");
					return;
				}
			}
			else {  /* something went wrong */
				Logging::bug("Skill quest not found", 0);
				sq_cleanup(ch);
				return;
			}

			reward = number_range(ch->level / 4, ch->level * 3 / 2) + 1;
			
			if (quest_double)
				pointreward = number_range(1, 20);
			else
				pointreward = number_range(1, 10);
			
			/* Awards ceremony */
			do_say(questman, "Congratulations on completing your skill quest!");
			Format::sprintf(buf, "As a reward, I am giving you %d skill point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), reward);
			do_say(questman, buf);

	                if (!ch->revoke_flags.has(REVOKE_EXP)) {
				int xp = number_range(100, 300);
		                ptc(ch, "{PYou receive %d experience points.{x\n", xp);
        	                gain_exp(ch, xp);
			}

			sn = get_random_skill(ch);

			if (chance(20) && sn != -1) {
				Format::sprintf(buf, "I will also teach you some of the finer points of %s.", skill_table[sn].name);
				do_say(questman, buf);
				ptc(ch, "%s helps you practice %s.\n", questman->short_descr, skill_table[sn].name);
				check_improve(ch, sn, TRUE, -1); /* always improve */
			}

			sq_cleanup(ch);
			ch->pcdata->nextsquest = pointreward;
			ch->gold += reward;
			ch->pcdata->skillpoints += pointreward;
			wiznet("{Y:SKILL QUEST:{x $N has completed a skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}
		else if ((questman = find_questmaster(ch)) != nullptr) {
			act("$n informs $N $e has completed $s quest.", ch, nullptr, questman, TO_ROOM);
			act("You inform $N you have completed your quest.", ch, nullptr, questman, TO_CHAR);

			if (ch->quest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!ch->act_flags.has(PLR_QUESTOR)) {
				do_say(questman, "But you aren't on a quest at the moment!");
				Format::sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
				do_say(questman, buf);
				/* clean up, just in case */
				ch->questmob = 0;
				ch->questobj = 0;
				ch->questobf = 0;
				ch->questloc = 0;
				ch->countdown = 0;
				return;
			}

			if (ch->nextquest > 0 || ch->countdown <= 0) {
				do_say(questman, "But you didn't complete your quest in time!");
				/* clean up, just in case */
				ch->questmob = 0;
				ch->questobj = 0;
				ch->questobf = 0;
				ch->questloc = 0;
				ch->countdown = 0;
				return;
			}

			/* check if player has the quest object */
			if (ch->questobj > 0) {
				for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
					if (obj->pIndexData == nullptr) {
						Logging::bug("QUEST COMPLETE: obj with nullptr pIndexData", 0);
						obj = nullptr;
						break;
					}
					else if (obj->pIndexData->vnum == ch->questobj)
						break;
				}
			}

			/* Added this in cause they drop their quest item after finding it */
			if (ch->questobf < 0 && obj == nullptr) {
				do_say(questman, "You must have lost your quest item on the way here.  Hurry and find it!");
				ch->questobf = 0;
				return;
			}

			/* check for incomplete quest */
			if (ch->questmob > 0 || (ch->questobj > 0 && obj == nullptr)) {
				do_say(questman, "You haven't completed the quest yet, but there is still time!");
				return;
			}

			/* rewards are calculated in the separate cases for mob and obj, but the ceremony is left
			   until after the endif to avoid duplicate code -- Elrac */
			if (ch->questobj > 0 && obj != nullptr) {
				act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
				act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);
				extract_obj(obj);
				obj = nullptr;
			}
			else if (ch->questmob != -1) {
				/* we shouldn't be here. */
				Logging::bug("QUEST COMPLETE: at reward phase without kill or object", 0);
				return;
			}

			reward = number_range(ch->level / 2, ch->level * 5 / 2) + 1; /* +1 is for the off chance of a level 0 questor */

			if (quest_double)
				pointreward = number_range(1, 10);
			else
				pointreward = number_range(1, 5);

			if (chance(5))
				pracreward = number_range(1, 3);

			/* Awards ceremony */
			do_say(questman, "Congratulations on completing your quest!");
			Format::sprintf(buf, "As a reward, I am giving you %d quest point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), reward);
			do_say(questman, buf);

	                if (!ch->revoke_flags.has(REVOKE_EXP)) {
				int xp = number_range(100, 300);
		                ptc(ch, "{PYou receive %d experience points.{x\n", xp);
        	                gain_exp(ch, xp);
			}

			if (pracreward > 0)
				ptc(ch, "{YYou also gain %d practice%s!{x\n", pracreward, (pracreward == 1 ? "" : "s"));

			ch->act_flags -= PLR_QUESTOR;
			ch->quest_giver = 0;
			ch->countdown = 0;
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;

			if (quest_double)
				ch->nextquest = UMAX(1, pointreward/2);
			else
				ch->nextquest = pointreward;

			ch->gold += reward;
			ch->questpoints += pointreward;
			ch->practice += pracreward;
			wiznet("{Y:QUEST:{x $N has completed a quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}
		else
			stc("You can't do that here.\n", ch);

		return;
	}

	/*** CLOSE ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("close")) {
		int num_in_area;
		int num_to_oust = 0;
		Character *victim;
		RoomPrototype *temple;

		if (!Game::world().quest.open) {
			stc("The quest area is not currently open.\n", ch);
			return;
		}

		String num_arg;
		argument = one_argument(argument, num_arg);

		if (!num_arg.empty() && num_arg.is_number())
			num_to_oust = atoi(num_arg);

		num_in_area = Game::world().quest.area->nplayer;

		if (num_in_area > 0) {
			if (num_to_oust < num_in_area - 1 || num_to_oust > num_in_area + 1) {
				ptc(ch, "There are still %d players in the quest area.\n", num_in_area);
				stc("Close the quest on how many players?\n", ch);
				return;
			}

			temple = get_room_index(ROOM_VNUM_TEMPLE);

			if (temple == nullptr)
				Logging::bug("QUEST CLOSE: Temple location not found (%d)", ROOM_VNUM_TEMPLE);
			else {
				for (victim = char_list; victim != nullptr; victim = victim->next) {
					if (!IS_NPC(victim) && victim->in_room != nullptr
					    && victim->in_room->area == Game::world().quest.area) {
						act("You expel $N from the quest area.", ch, nullptr, victim, TO_CHAR);
						stc("You are expelled from the quest area.\n", victim);
						char_from_room(victim);
						char_to_room(victim, temple);
						act("$n arrives from a mushroom cloud.", victim, nullptr, nullptr, TO_ROOM);
						do_look(victim, "auto");
					}
				}
			}
		}

		stc("*** You have closed the quest area ***\n", ch);
		Format::sprintf(buf, "%s has closed the quest area.\n", ch->name);
		do_send_announce(ch, buf);
		Game::world().quest.open = FALSE;
		return;
	}

	/*** DEDUCT ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("deduct")) {
		Character *victim;
		int qpoint;

		String arg;
		argument = one_argument(argument, arg);

		if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
			stc("Player not found.\n", ch);
			return;
		}

		if (!argument.is_number()) {
			stc("Value must be numeric.\n", ch);
			return;
		}

		qpoint = atoi(argument);

		if (qpoint < 0) {
			stc("Value must be positive.\n", ch);
			return;
		}

		if (qpoint > victim->questpoints) {
			stc("They do not have enough quest points for that.\n", ch);
			return;
		}

		victim->questpoints -= qpoint;
		Format::sprintf(buf, "%d questpoints have been deducted, %d remaining.\n", qpoint, victim->questpoints);
		stc(buf, ch);
		stc(buf, victim);
		Format::sprintf(buf, "%s deducted %d qp from %s [%d remaining].", ch->name, qpoint, victim->name, victim->questpoints);
		wiznet(buf, ch, nullptr, WIZ_SECURE, WIZ_QUEST, GET_RANK(ch));
		return;
	}

	/*** DOUBLE ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("double")) {
		if (!quest_double) {
			stc("You declare double QP for all!\n", ch);
			wiznet("{Y:QUEST:{x $N has declared double QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared Double QP for all!  Happy Questing!");
			quest_double = 1;
		}
		else {
			stc("You declare normal QP for all.\n", ch);
			wiznet("{Y:QUEST:{x $N has declared normal QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared normal questing.");
			quest_double = 0;
		}

		return;
	}

	/*** FORFEIT ***/
	if (arg1.is_prefix_of("forfeit")) {
		if (!ch->act_flags.has(PLR_QUESTOR) && !ch->pcdata->plr_flags.has(PLR_SQUESTOR)) {
			stc("You aren't currently on a quest.\n", ch);
			return;
		}

		if (ch->pcdata->plr_flags.has(PLR_SQUESTOR) && find_squestmaster(ch) != nullptr) {
			sq_cleanup(ch);
			ch->pcdata->nextsquest = 10;
			wiznet("{Y:SKILL QUEST:{x $N has forfeited $S skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your skill quest.\n", ch);
			return;
		}

		if (ch->act_flags.has(PLR_QUESTOR) && find_questmaster(ch) != nullptr) {
			ch->act_flags -= PLR_QUESTOR;
			ch->quest_giver = 0;
			ch->countdown = 0;
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;
			ch->nextquest = 6;
			wiznet("{Y:QUEST:{x $N has forfeited $S quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your quest.\n", ch);
			return;
		}

		stc("You can't do that here.\n", ch);
		return;
	}

	/*** INFO ***/
	if (arg1.is_prefix_of("info")) {
		if (ch->in_room == nullptr || ch->in_room->area == nullptr) {
			stc("You cannot recall your quest from this location.\n", ch);
			return;
		}

		squest_info(ch);
		stc("\n", ch);
		quest_info(ch);
		return;
	}

	/*** JOIN ***/
	if (arg1.is_prefix_of("join")) {
		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if (ch->in_room == nullptr) {
			stc("Something prevents you from joining the quest.\n", ch);
			return;
		}

		if (!Game::world().quest.open) {
			stc("Sorry, but the quest area has not been opened!\n", ch);
			return;
		}

		if (ch->level < Game::world().quest.min_level || ch->level > Game::world().quest.max_level) {
			stc("Sorry, but the quest area is not open to your level!\n", ch);
			return;
		}

		if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_NO_RECALL)) {
			stc("You cannot join the quest from this location.\n", ch);
			return;
		}

		if (affect_exists_on_char(ch, gsn_curse)) {
			stc("You cannot join the quest in your current contition.\n", ch);
			return;
		}

		if (ch->fighting != nullptr) {
			stc("You will need to stop fighting before you can join the quest.\n", ch);
			return;
		}

		act("$n joins the quest!", ch, nullptr, nullptr, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, Game::world().quest.startroom);
		act("$n appears in the room.", ch, nullptr, nullptr, TO_ROOM);
		stc("You join the Quest!\n", ch);
		do_look(ch, "auto");
		return;
	}

	/*** LIST ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("list")) {
		char qblock[MAX_STRING_LENGTH], sqblock[MAX_STRING_LENGTH], mblock[MAX_STRING_LENGTH],
		     oblock[MAX_STRING_LENGTH], lblock[MAX_STRING_LENGTH];
		String output;
		Descriptor *d;
		stc("                            {YQuest                         {GSkill Quest{x\n", ch);
		stc("Name           Mobile Object Loctn   Time  QP   --- MobLoc ObjLoc  Time  SP\n", ch);
		stc("-----------------------------------------------------------------------------\n", ch);

		for (d = descriptor_list; d != nullptr; d = d->next) {
			Character *wch;

			if (!IS_PLAYING(d) || !can_see_who(ch, d->character))
				continue;

			wch = (d->original != nullptr) ? d->original : d->character;

			if (IS_NPC(wch) || !can_see_who(ch, wch))
				continue;

			if (IS_QUESTOR(wch)) {
				if (wch->questobj > 0) {                /* an item quest */
					Format::sprintf(mblock, "{Y<%5d>{x", 0);

					if (wch->questobf < 0) {        /* item has been found */
						Format::sprintf(oblock, "{C<{Yfound{C>{x");
						Format::sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* item has not been found */
						Format::sprintf(oblock, "{T<{Y%5d{T>{x", wch->questobj);
						Format::sprintf(lblock, "{T<{Y%5d{T>{x", wch->questloc);
					}
				}
				else {                                  /* a mob quest */
					Format::sprintf(oblock, "{Y<%5d>{x", 0);

					if (wch->questmob == -1) {      /* mob has been killed */
						Format::sprintf(mblock, "{C<{Y dead{C>{x");
						Format::sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* mob has not been killed */
						Format::sprintf(mblock, "{T<{Y%5d{T>{x", wch->questmob);
						Format::sprintf(lblock, "{T<{Y%5d{T>{x", wch->questloc);
					}
				}

				Format::sprintf(qblock, "%s%s%s {Y[%2d][%4d]{x",
				        mblock, oblock, lblock, wch->countdown, wch->questpoints);
			}
			else
				Format::sprintf(qblock, "<%5d><%5d><%5d> [%2d][%4d]",
				        0, 0, 0, wch->nextquest, wch->questpoints);

			if (IS_SQUESTOR(wch)) {
				if (wch->pcdata->squestmob == nullptr      /* item quest */
				    && wch->pcdata->squestobj != nullptr) {
					Format::sprintf(mblock, "{G<%5d>{x", 0);

					if (wch->pcdata->squestobjf)    /* item has been found */
						Format::sprintf(oblock, "{C<{Gfound{C>{x");
					else
						Format::sprintf(oblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc1);
				}
				else if (wch->pcdata->squestobj == nullptr /* mob quest */
				         && wch->pcdata->squestmob != nullptr) {
					Format::sprintf(oblock, "{G<%5d>{x", 0);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						Format::sprintf(mblock, "{C<{Gfound{C>{x");
					else
						Format::sprintf(mblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc2);
				}
				else if (wch->pcdata->squestobj != nullptr /* item to mob quest */
				         && wch->pcdata->squestmob != nullptr) {
					if (wch->pcdata->squestobjf)    /* item has been found */
						Format::sprintf(oblock, "{C<{Gfound{C>{x");
					else
						Format::sprintf(oblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc1);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						Format::sprintf(mblock, "{C<{Gfound{C>{x");
					else
						Format::sprintf(mblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc2);
				}
				else {                                  /* shouldn't be here */
					Format::sprintf(oblock, "{P<? ? ? ? ?>{x");
					Format::sprintf(mblock, "{P<? ? ? ? ?>{x");
				}

				Format::sprintf(sqblock, "%s%s {G[%2d][%4d]{x",
				        mblock, oblock, wch->pcdata->sqcountdown, wch->pcdata->skillpoints);
			}
			else
				Format::sprintf(sqblock, "<%5d><%5d> [%2d][%4d]",
				        0, 0, wch->pcdata->nextsquest, wch->pcdata->skillpoints);

			Format::sprintf(buf, "%-14s %s     %s{x\n", wch->name, qblock, sqblock);
			output += buf;
		}

		page_to_char(output, ch);
		return;
	}

	/*** OPEN ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("open")) {
		if (Game::world().quest.startroom == nullptr) {
			stc("The quest area is not available in this reboot.\n", ch);
			return;
		}

		Game::world().quest.area = Game::world().quest.startroom->area;

		if (Game::world().quest.open) {
			ptc(ch, "The quest area is already open, to levels %d to %d\n", Game::world().quest.min_level, Game::world().quest.max_level);
			return;
		}

		String num_arg;
		argument = one_argument(argument, num_arg);

		if (!num_arg.empty() && num_arg.is_number())
			Game::world().quest.min_level = atoi(num_arg);

		if (Game::world().quest.min_level < 1 || Game::world().quest.min_level > 100) {
			stc("Open the quest to which minimum level (1..100) ?\n", ch);
			return;
		}

		argument = one_argument(argument, num_arg);

		if (!num_arg.empty() && num_arg.is_number())
			Game::world().quest.max_level = atoi(num_arg);

		if (Game::world().quest.max_level < Game::world().quest.min_level || Game::world().quest.max_level > 100) {
			ptc(ch, "Open the quest for levels %d to which maximum level (%d..100) ?\n",
			    Game::world().quest.min_level, Game::world().quest.min_level);
			return;
		}

		Format::sprintf(buf, "%s has opened the quest area to levels %d through %d!\n", ch->name, Game::world().quest.min_level, Game::world().quest.max_level);
		do_send_announce(ch, buf);
		ptc(ch, "You open the quest area to levels %d through %d.\n", Game::world().quest.min_level, Game::world().quest.max_level);
		Game::world().quest.open = TRUE;
		return;
	}

	/*** PK ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("pk")) {
		Character *salesgnome;
		RoomPrototype *to_room;
		salesgnome = get_mob_world(ch, "salesgnome", VIS_CHAR);

		if (salesgnome == nullptr)
			Logging::bug("QUEST PK: salesgnome not there", 0);

		stc("PK in Questlands is now ", ch);
		Game::world().quest.pk = !Game::world().quest.pk;

		if (Game::world().quest.pk) {
			stc("UNLIMITED (Questlands = Open Arena)\n", ch);
			wiznet("{Y:QUEST:{x $N has opened Questlands for Open Arena", ch, nullptr, WIZ_QUEST, 0, 0);

			if (salesgnome != nullptr) {
				to_room = get_room_index(ROOM_VNUM_ARENATICKET);

				if (to_room == nullptr) {
					Logging::bug("QUEST PK: Can't find ticket booth", 0);
					return;
				}

				char_from_room(salesgnome);
				char_to_room(salesgnome, to_room);
			}
		}
		else {
			stc("LIMITED (like everywhere else)\n", ch);
			wiznet("{Y:QUEST:{x $N has restricted PK in Questlands", ch, nullptr, WIZ_QUEST, 0, 0);

			if (salesgnome != nullptr) {
				to_room = get_room_index(ROOM_VNUM_TICKETBACKROOM);

				if (to_room == nullptr) {
					Logging::bug("QUEST PK: Can't find ticket booth back room", 0);
					return;
				}

				char_from_room(salesgnome);
				char_to_room(salesgnome, to_room);
			}
		}

		return;
	}

	/*** POINTS ***/
	if (arg1.is_prefix_of("points")) {
		ptc(ch, "You have %d quest points and %d skill points.\n",
		    ch->questpoints, ch->pcdata->skillpoints);
		return;
	}

	/*** REQUEST ***/
	if (arg1.is_prefix_of("request")) {
		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if ((questman = find_squestmaster(ch)) != nullptr) {
			act("$n asks $N for a skill quest.", ch, nullptr, questman, TO_ROOM);
			act("You ask $N for a skill quest.", ch, nullptr, questman, TO_CHAR);

			if (IS_NPC(ch) && ch->act_flags.has(ACT_PET)) {
				check_social(questman, "rofl", ch->name.c_str());

				Format::sprintf(buf, "Who ever heard of a pet questing for its %s?",
				        GET_ATTR_SEX(ch) == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != nullptr)
					check_social(questman, "laugh", ch->leader->name.c_str());

				return;
			}

			if (ch->pcdata->plr_flags.has(PLR_SQUESTOR)) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->pcdata->nextsquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'",
				    ch, nullptr, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'",
				    ch, nullptr, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_CHAR);
			sq_cleanup(ch);
			generate_skillquest(ch, questman);
			ch->pcdata->plr_flags += PLR_SQUESTOR;
			Format::sprintf(buf, "You have %d minutes to complete this quest.", ch->pcdata->sqcountdown);
			do_say(questman, buf);
			do_say(questman, "May the gods go with you!");
			wiznet("{Y:SKILL QUEST:{x $N has begun a skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}

		if ((questman = find_questmaster(ch)) != nullptr) {
			act("$n asks $N for a quest.", ch, nullptr, questman, TO_ROOM);
			act("You ask $N for a quest.", ch, nullptr, questman, TO_CHAR);

			if (IS_NPC(ch) && ch->act_flags.has(ACT_PET)) {
				check_social(questman, "rofl", ch->name.c_str());

				Format::sprintf(buf, "Who ever heard of a pet questing for its %s?",
				        GET_ATTR_SEX(ch) == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != nullptr)
					check_social(questman, "laugh", ch->leader->name.c_str());

				return;
			}

			if (ch->act_flags.has(PLR_QUESTOR)) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->nextquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_CHAR);
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;
			generate_quest(ch, questman);

			if (ch->questmob > 0 || ch->questobj > 0) {
				ch->act_flags += PLR_QUESTOR;
				Format::sprintf(buf, "You have %d minutes to complete this quest.", ch->countdown);
				do_say(questman, buf);
				do_say(questman, "May the gods go with you!");
			}

			wiznet("{Y:QUEST:{x $N has begun a quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}
		else
			stc("You can't do that here.\n", ch);

		return;
	}

	/*** STATUS ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("status")) {
		if (Game::world().quest.open)
			stc("The Quest Area is currently open for a {Rquest{x.\n", ch);
		else
			stc("There is currently {Gno quest{x going on.\n", ch);

		if (Game::world().quest.pk)
			stc("There is {Gunrestricted carnage{x in the Quest Area.\n", ch);
		else
			stc("The Quest Area is currently PK {Rrestricted{x.\n", ch);

		return;
	}

	/*** TIME ***/
	if (arg1.is_prefix_of("time")) {
		if (!ch->act_flags.has(PLR_QUESTOR)) {
			stc("You aren't currently on a quest.\n", ch);

			if (ch->nextquest > 1)
				ptc(ch, "There are %d minutes remaining until you can quest again.\n",
				    ch->nextquest);
			else if (ch->nextquest == 1)
				stc("There is less than a minute remaining until you can quest again.\n", ch);
		}
		else if (ch->countdown > 0)
			ptc(ch, "You have %d minutes left to complete your current quest.\n", ch->countdown);

		if (!ch->pcdata->plr_flags.has(PLR_SQUESTOR)) {
			stc("You aren't currently on a skill quest.\n", ch);

			if (ch->pcdata->nextsquest > 1)
				ptc(ch, "There are %d minutes remaining until you can go on another skill quest.\n",
				    ch->pcdata->nextsquest);
			else if (ch->pcdata->nextsquest == 1)
				stc("There is less than a minute remaining until you can go on another skill quest.\n", ch);
		}
		else if (ch->pcdata->sqcountdown > 0)
			ptc(ch, "You have %d minutes left to complete your current skill quest.\n",
			    ch->pcdata->sqcountdown);

		return;
	}

	/* Unknown keyword, or not within character's level */
	quest_usage(ch);
}

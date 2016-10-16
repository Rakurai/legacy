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

#include "merc.h"
#include "tables.h"
#include "recycle.h"

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

/* The Vnum of the quest start location -- hard coded! */
#define QUEST_STARTROOM 12000

void quest_init(void)
{
	quest_open = FALSE;
	quest_upk = TRUE;   /* unlimited PK */
	quest_startroom = get_room_index(QUEST_STARTROOM);

	if (quest_startroom == NULL) {
		bug("quest_init: Can't find quest start room %d.", QUEST_STARTROOM);
		quest_area = NULL;
	}
	else
		quest_area = quest_startroom->area;
} /* end quest_init */

/* Usage info on the QUEST commands -- Elrac */
/* Keep this in line with the do_quest function's keywords */
void quest_usage(CHAR_DATA *ch)
{
	set_color(ch, YELLOW, BOLD);
	stc("QUEST commands: REQUEST INFO TIME COMPLETE FORFEIT POINTS JOIN.\n", ch);

	if (IS_IMMORTAL(ch))
		stc("Imm QUEST commands: OPEN CLOSE LIST DOUBLE DEDUCT PK.\n", ch);

	stc("For more information, type 'HELP QUEST'.\n", ch);
	set_color(ch, WHITE, NOBOLD);
} /* end quest_usage */

void sq_cleanup(CHAR_DATA *ch)
{
	CHAR_DATA *mob;
	OBJ_DATA *obj;
	REMOVE_BIT(ch->pcdata->plr, PLR_SQUESTOR);
	ch->pcdata->squest_giver = 0;
	ch->pcdata->sqcountdown = 0;

	for (obj = object_list; obj != NULL; obj = obj->next)
		if (obj == ch->pcdata->squestobj)
			extract_obj(obj);

	for (mob = char_list; mob != NULL ; mob = mob->next)
		if (mob == ch->pcdata->squestmob)
			extract_char(mob, TRUE);

	ch->pcdata->squestobj = NULL;
	ch->pcdata->squestmob = NULL;
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
CHAR_DATA *find_questmaster(CHAR_DATA *ch)
{
	CHAR_DATA *questman;

	for (questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room) {
		if (!IS_NPC(questman))
			continue;

		if (questman->spec_fun == spec_lookup("spec_questmaster"))
			break;
	}

	if (questman == NULL)
		return NULL;

	if (questman->pIndexData == NULL) {
		bug("find_questmaster: Questmaster has NULL pIndexData!", 0);
		questman = NULL;
	}

	if (questman->fighting != NULL) {
		stc("Wait until the fighting stops.\n", ch);
		questman = NULL;
	}

	return questman;
} /* end find_questmaster */

CHAR_DATA *find_squestmaster(CHAR_DATA *ch)
{
	CHAR_DATA *questman;

	for (questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room) {
		if (!IS_NPC(questman))
			continue;

		if (questman->spec_fun == spec_lookup("spec_squestmaster"))
			break;
	}

	if (questman == NULL)
		return NULL;

	if (questman->pIndexData == NULL) {
		bug("find_questmaster: Questmaster has NULL pIndexData!", 0);
		questman = NULL;
	}

	if (questman->fighting != NULL) {
		stc("Wait until the fighting stops.\n", ch);
		questman = NULL;
	}

	return questman;
} /* end find_squestmaster */

/* Obtain additional location information about sought item/mob */
void quest_where(CHAR_DATA *ch, char *what)
{
	ROOM_INDEX_DATA *room;

	if (ch->questloc <= 0) {
		bug("QUEST INFO: ch->questloc = %d", ch->questloc);
		return;
	}

	if (ch->in_room == NULL)
		return;

	room = get_room_index(ch->questloc);

	if (room->area == NULL) {
		bug("QUEST INFO: room(%d)->area == NULL", ch->questloc);
		return;
	}

	if (room->area->name == NULL) {
		bug("QUEST INFO: area->name == NULL", 0);
		return;
	}

	ptc(ch, "Rumor has it this %s was last seen in the area known as %s,\n", what, room->area->name);

	if (room->name == NULL) {
		bug("QUEST INFO: room(%d)->name == NULL", ch->questloc);
		return;
	}

	ptc(ch, "near %s.\n", room->name);
} /* end quest_where */

void squest_info(CHAR_DATA *ch)
{
	MOB_INDEX_DATA *questman;
	ROOM_INDEX_DATA *questroom_obj, *questroom_mob;

	if (!IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
		stc("You aren't currently on a skill quest.\n", ch);
		return;
	}

	if (ch->pcdata->squest_giver < 1) {
		bug("QUEST INFO: quest giver = %d", ch->pcdata->squest_giver);
		stc("It seems the questmistress died of old age waiting for you.\n", ch);
		REMOVE_BIT(ch->pcdata->plr, PLR_SQUESTOR);
		return;
	}

	questman = get_mob_index(ch->pcdata->squest_giver);

	if (questman == NULL) {
		bug("QUEST INFO: skill quest giver %d has no MOB_INDEX_DATA!", ch->quest_giver);
		stc("The questmistress has fallen very ill. Please contact an imm!\n", ch);
		REMOVE_BIT(ch->pcdata->plr, PLR_SQUESTOR);
		return;
	}

	if (ch->pcdata->squestobj == NULL && ch->pcdata->squestmob == NULL) { /* no quest */
		stc("You've forgotten what your skill quest was.\n", ch);
		bug("QUEST INFO: skill quest with no obj or mob", 0);
		sq_cleanup(ch);
		return;
	}

	ptc(ch, "You recall the skill quest which %s gave you.\n", questman->short_descr);

	if (ch->pcdata->squestobj != NULL && ch->pcdata->squestmob == NULL) { /* obj, no mob */
		if (ch->pcdata->squestobjf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr == NULL ? "your quest master" : questman->short_descr));
			return;
		}

		if ((questroom_obj = get_room_index(ch->pcdata->squestloc1)) == NULL) {
			bug("QUEST INFO: sqobj quest with no location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_obj->area == NULL) {
			bug("QUEST INFO: sqobj location with no area", 0);
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

	if (ch->pcdata->squestobj == NULL && ch->pcdata->squestmob != NULL) { /* mob, no obj */
		if (ch->pcdata->squestmobf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr == NULL ? "the questmistress" : questman->short_descr));
			return;
		}

		if ((questroom_mob = get_room_index(ch->pcdata->squestloc2)) == NULL) {
			bug("QUEST INFO: sqmob quest with no location", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_mob->area == NULL) {
			bug("QUEST INFO: sqmob location with no area", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		ptc(ch, "You are on a quest to learn from the legendary %s!\n",
		    ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    ch->pcdata->squestmob->sex == 1 ? "He" : "She", questroom_mob->name);
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
		return;
	}

	if (ch->pcdata->squestobj != NULL && ch->pcdata->squestmob != NULL) { /* mob and obj */
		if ((questroom_obj = get_room_index(ch->pcdata->squestloc1)) == NULL) {
			bug("QUEST INFO: sqobj/mob quest with no obj location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_obj->area == NULL) {
			bug("QUEST INFO: sqobj location with no area", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if ((questroom_mob = get_room_index(ch->pcdata->squestloc2)) == NULL) {
			bug("QUEST INFO: sqobj/mob quest with no mob location", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (questroom_mob->area == NULL) {
			bug("QUEST INFO: sqmob location with no area", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if (ch->pcdata->squestobjf && ch->pcdata->squestmobf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr == NULL ? "the questmistress" : questman->short_descr));
			return;
		}

		if (!ch->pcdata->squestmobf && ch->pcdata->squestobjf) {
			ptc(ch, "You must return the %s to %s.\n",
			    ch->pcdata->squestobj->short_descr, ch->pcdata->squestmob->short_descr);
			ptc(ch, "%s can usually be found in %s{x,\n",
			    ch->pcdata->squestmob->sex == 1 ? "He" : "She", questroom_mob->name);
			ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s,\n", ch->pcdata->squestobj->short_descr);
		ptc(ch, "from %s{x, in the area known as %s{x,\n", questroom_obj->name, questroom_obj->area->name);
		ptc(ch, "and return it to it's rightful owner, %s.\n", ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    ch->pcdata->squestmob->sex == 1 ? "He" : "She", questroom_mob->name);
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area->name);
	}
}

void quest_info(CHAR_DATA *ch)
{
	MOB_INDEX_DATA *questman, *questinfo;
	OBJ_INDEX_DATA *questinfoobj;

	if (!IS_SET(ch->act, PLR_QUESTOR)) {
		stc("You aren't currently on a quest.\n", ch);
		return;
	}

	if (ch->quest_giver < 1) {
		bug("QUEST INFO: quest giver = %d", ch->quest_giver);
		stc("It seems the questmaster died of old age waiting for you.\n", ch);
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		return;
	}

	questman = get_mob_index(ch->quest_giver);

	if (questman == NULL) {
		bug("QUEST INFO: quest giver %d has no MOB_INDEX_DATA!", ch->quest_giver);
		stc("The questmaster has fallen very ill. Please contact an imm!\n", ch);
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		return;
	}

	if (ch->questmob == -1) { /* killed target mob */
		stc("Your quest is ALMOST complete!\n", ch);
		ptc(ch, "Get back to %s before your time runs out!\n",
		    (questman->short_descr == NULL ? "your quest master" : questman->short_descr));
		return;
	}
	else if (ch->questobj > 0) { /* questing for an object */
		questinfoobj = get_obj_index(ch->questobj);

		if (questinfoobj != NULL) {
			stc("You recall the quest which the questmaster gave you.\n", ch);
			ptc(ch, "You are on a quest to recover the fabled %s!\n", questinfoobj->name);
			quest_where(ch, "treasure");
			return;
		}

		/* quest object not found! */
		bug("No info for quest object %d", ch->questobj);
		ch->questobj = 0;
		ch->questobf = 0;
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		/* no RETURN -- fall thru to 'no quest', below */
	}
	else if (ch->questmob > 0) { /* questing for a mob */
		questinfo = get_mob_index(ch->questmob);

		if (questinfo != NULL) {
			ptc(ch, "You are on a quest to slay the dreaded %s!\n", questinfo->short_descr);
			quest_where(ch, "fiend");
			return;
		}

		/* quest mob not found! */
		bug("No info for quest mob %d", ch->questmob);
		ch->questmob = 0;
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		/* no RETURN -- fall thru to 'no quest', below */
	}

	/* we shouldn't be here */
	bug("QUEST INFO: Questor with no kill, mob or obj", 0);
	return;
}

int get_random_skill(CHAR_DATA *ch)
{
	int count;
	int sn = 0;
	int pass = 1;
	int target = 0;

	/* pick a random skill or spell, if they have any */
	while (pass != 0) {
		count = 0;

		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name != NULL) {
				if (ch->pcdata->learned[sn] <= 0
				    || ch->pcdata->learned[sn] >= 100)
					continue;

				if (skill_table[sn].remort_class > 0 && !CAN_USE_RSKILL(ch, sn))
					continue;

				if (skill_table[sn].skill_level[ch->class] > ch->level)
					continue;

				if (pass == 2 && count == target) {
					pass = 0;               /* get out of while loop */
					break;                  /* get out of for loop */
				}

				count++;
			}
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

void squestobj_to_squestmob(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *mob)
{
	char buf[MAX_STRING_LENGTH];
	check_social(mob, "beam", ch->name);
	sprintf(buf, "Thank you, %s, for returning my lost %s!", ch->name, obj->short_descr);
	do_say(mob, buf);
	extract_obj(obj);
	do_say(mob, "I left the gold reward with the Questmistress.  Farewell, and thank you!");
	act("$n turns and walks away.", mob, NULL, NULL, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	sprintf(buf, "{Y:SKILL QUEST: {x$N has returned %s to %s", obj->short_descr, mob->short_descr);
	wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
	extract_char(mob, TRUE);
	ch->pcdata->squestmobf = TRUE;
}

void squestmob_found(CHAR_DATA *ch, CHAR_DATA *mob)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->pcdata->squestmobf) {
		bug("At squestmob_found, player's squestmob already found.  Continuing...", 0);
		ch->pcdata->squestmobf = FALSE;
	}

	if (IS_SET(mob->act, ACT_MAGE)) {
		switch (number_range(1, 1)) {
		case 1:
			do_say(mob, "Greetings, young spellcaster!");
			do_say(mob, "I will teach you about magic in it's most basic form.");
			do_say(mob, "It is important to concentrate on clearly envisioning\n"
			       "the forming spell in your mind.  Simple spells require less\n"
			       "thought, and have more room for error, but focusing on the\n"
			       "spell, shaping the magic in your mind, perfecting it, is what\n"
			       "sets apart the spellcaster from the wizard.");
			act("$N leads you through some simple exercises to clear your mind,", ch, NULL, mob, TO_CHAR);
			act("then helps you to visualize the abstract logic required to form", ch, NULL, mob, TO_CHAR);
			act("a fireball.", ch, NULL, mob, TO_CHAR);
			act("$N touches a finger to $n's head, and $e looks more relaxed.", ch, NULL, mob, TO_NOTVICT);
			do_say(mob, "Any mage can cast a simple fireball.  But a true student of\n"
			       "magic will look for new ways, and cast a more powerful spell in the end.");
			act("In your mind, the fireball starts to glow brighter, it's color", ch, NULL, mob, TO_CHAR);
			act("turning to a soft green.", ch, NULL, mob, TO_CHAR);
			act("Beads of sweat start to run down $n's forehead as $e concentrates.", ch, NULL, mob, TO_NOTVICT);
			do_say(mob, "The truly great magicians strive to make spells their own.");
			act("The fireball in your mind fades.", ch, NULL, mob, TO_CHAR);
			act("$n opens $s eyes.", ch, NULL, mob, TO_NOTVICT);
			break;
		}
	}
	else if (IS_SET(mob->act, ACT_CLERIC)) {
		switch (number_range(1, 1)) {
		case 1:
			act("$N smiles warmly at $n.", ch, NULL, mob, TO_NOTVICT);
			act("$N smiles warmly at you.", ch, NULL, mob, TO_CHAR);
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
	else if (IS_SET(mob->act, ACT_THIEF)) {
		switch (number_range(1, 1)) {
		case 1:
			ptc(ch, "You feel a tap on your shoulder, and turn to see %s.\n", mob->short_descr);
			do_say(mob, "Many people express distaste at the life of a lowly pickpocket.");
			do_say(mob, "But, few occupations require such skill, dexterity, and daring.");
			act("$n starts into a long monologue detailing the ethics of theft for a living.", mob, NULL, NULL, TO_ROOM);
			do_say(mob, "Is it right to steal?  I'll let you decide for yourself, no one\n"
			       "  needs another to explain moral values.  But, for me...");
			act("$N hands $n $s gold pouch, and winks.", ch, NULL, mob, TO_NOTVICT);
			act("$N hands you your gold pouch, and winks.", ch, NULL, mob, TO_CHAR);
			break;
		}
	}
	else {  /* all sqmobs should have one flag, revert to warrior just in case */
		switch (number_range(1, 2)) {
		case 1:
			act("$n draws $s blade and places it over $s shoulder.", mob, NULL, NULL, TO_ROOM);
			do_say(mob, "The blade is of the most powerful of weapons.");
			do_say(mob, "It can be used for close combat, distanced, and defense.");
			act("$n takes a fighting stance, raising $s blade into the air.", mob, NULL, NULL, TO_ROOM);
			do_say(mob, "In one fluid movement one can overcome an untrained oppenent.");
			act("$n whirls the blade down swiftly, pulls it back up into\n"
			    " the air and slices down again.", mob, NULL, NULL, TO_ROOM);
			do_say(mob, "A blade can be used for good, or for evil.");
			act("$n pulls the blade back up and slides it into its sheath.", mob, NULL, NULL, TO_ROOM);
			do_say(mob, "But its not the blade who decides...");
			break;

		case 2:
			act("$N draws $S blade and motions toward $n.", ch, NULL, mob, TO_NOTVICT);
			act("$N draws $S blade and motions toward you.", ch, NULL, mob, TO_CHAR);
			do_say(mob, "A blade is true, unyeilding, and unstoppable.");
			do_say(mob, "It can be used to protect, to defend, to destroy, to disarm.");
			act("$N positions $S blade to point toward $n.", ch, NULL, mob, TO_NOTVICT);
			act("$N positions $S blade to point toward you.", ch, NULL, mob, TO_CHAR);
			do_say(mob, "The soul of a blade is within you, and only by your hand can it be awakened.");
			break;
		}
	}

	act("$n turns and walks away.", mob, NULL, NULL, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	sprintf(buf, "{Y:SKILL QUEST: {x$N has spoken with %s", mob->short_descr);
	wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
	extract_char(mob, TRUE);
	ch->pcdata->squestmobf = TRUE;
}

OBJ_DATA *generate_skillquest_obj(CHAR_DATA *ch, int level)
{
	EXTRA_DESCR_DATA *ed;
	OBJ_DATA *questobj;
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

		{NULL, NULL, NULL, NULL}
	};

	/* count the objects */
	for (num_objs = 0; sq_item_table[num_objs].name != NULL; num_objs++);

	num_objs--;
	descnum = number_range(0, num_objs);
	questobj = create_object(get_obj_index(OBJ_VNUM_SQUESTOBJ), level);

	if (! questobj) {
		bug("Memory error creating quest object.", 0);
		return NULL;
	}

	questobj->timer = (4 * ch->pcdata->sqcountdown + 10) / 3;
	free_string(questobj->name);
	free_string(questobj->short_descr);
	free_string(questobj->description);
	sprintf(buf, "%s %s", sq_item_table[descnum].name, ch->name);
	questobj->name        = str_dup(buf);
	questobj->short_descr = str_dup(sq_item_table[descnum].shortdesc);
	questobj->description = str_dup(sq_item_table[descnum].longdesc);
	ed = new_extra_descr();
	ed->keyword        = str_dup(sq_item_table[descnum].name);
	ed->description    = str_dup(sq_item_table[descnum].extended);
	ed->next           = questobj->extra_descr;
	questobj->extra_descr   = ed;
	ch->pcdata->squestobj = questobj;
	return questobj;
}

ROOM_INDEX_DATA *generate_skillquest_room(CHAR_DATA *ch, int level)
{
	ROOM_INDEX_DATA *room, *prev;

	for (; ;) {
		room = get_room_index(number_range(0, 32767));

		if (room == NULL
		    || !can_see_room(ch, room)
		    || room->area == quest_area
		    || room->area->low_range > level
		    || room->area->high_range < level
		    || (room->area->min_vnum >= 24000      /* clanhall vnum ranges */
		        && room->area->min_vnum <= 26999)
		    || room->guild
		    || !str_cmp(room->area->name, "Playpen")
		    || !str_cmp(room->area->name, "IMM-Zone")
		    || !str_cmp(room->area->name, "Limbo")
		    || !str_cmp(room->area->name, "Midgaard")
		    || !str_cmp(room->area->name, "New Thalos")
		    || !str_cmp(room->area->name, "Eilyndrae")     /* hack to make eilyndrae and torayna cri unquestable */
		    || !str_cmp(room->area->name, "Torayna Cri")
		    || !str_cmp(room->area->name, "Battle Arenas")
		    || room->sector_type == SECT_ARENA
		    || IS_SET(room->room_flags,
		              ROOM_MALE_ONLY
		              | ROOM_FEMALE_ONLY
		              | ROOM_PRIVATE
		              | ROOM_SOLITARY
		              | ROOM_NOQUEST))
			continue;

		/* no pet shops */
		if ((prev = get_room_index(room->vnum - 1)) != NULL)
			if (IS_SET(prev->room_flags, ROOM_PET_SHOP))
				continue;

		return room;
	}
}

void generate_skillquest_mob(CHAR_DATA *ch, CHAR_DATA *questman, int level, int type)
{
	OBJ_DATA  *questobj;
	CHAR_DATA *questmob;
	ROOM_INDEX_DATA *questroom;
	int x, maxnoun;
	char buf[MAX_STRING_LENGTH];
	char longdesc[MAX_STRING_LENGTH];
	char shortdesc[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char *title, *quest;
	title = quest = "";             /* ew :( */

	if ((questmob = create_mobile(get_mob_index(MOB_VNUM_SQUESTMOB))) == NULL) {
		bug("Bad skillquest mob vnum, from generate_skillquest_mob", 0);
		return;
	}

	questmob->sex = number_range(1, 2);
	questmob->level = ch->level;

	/* generate name */
	if (questmob->sex == 1)
		sprintf(shortdesc, "%s%s%s",
		        Msyl1[number_range(0, (MAXMSYL1 - 1))],
		        Msyl2[number_range(0, (MAXMSYL2 - 1))],
		        Msyl3[number_range(0, (MAXMSYL3 - 1))]);
	else
		sprintf(shortdesc, "%s%s%s",
		        Fsyl1[number_range(0, (MAXFSYL1 - 1))],
		        Fsyl2[number_range(0, (MAXFSYL2 - 1))],
		        Fsyl3[number_range(0, (MAXFSYL3 - 1))]);

	sprintf(name, "squestmob %s", shortdesc);
	free_string(questmob->name);
	free_string(questmob->short_descr);
	questmob->name = str_dup(name);
	questmob->short_descr = str_dup(shortdesc);

	/* generate title */
	switch (number_range(1, 4)) {
	case 1:
		for (maxnoun = 0; MagT_table[maxnoun].male != NULL; maxnoun++);

		x = number_range(0, --maxnoun);
		title = str_dup(questmob->sex == 1 ? MagT_table[x].male : MagT_table[x].female);
		quest = "the powers of magic";
		SET_BIT(questmob->act, ACT_MAGE);
		break;

	case 2:
		for (maxnoun = 0; CleT_table[maxnoun].male != NULL; maxnoun++);

		x = number_range(0, --maxnoun);
		title = str_dup(questmob->sex == 1 ? CleT_table[x].male : CleT_table[x].female);
		quest = "the wisdom of holiness";
		SET_BIT(questmob->act, ACT_CLERIC);
		break;

	case 3:
		for (maxnoun = 0; ThiT_table[maxnoun].male != NULL; maxnoun++);

		x = number_range(0, --maxnoun);
		title = str_dup(questmob->sex == 1 ? ThiT_table[x].male : ThiT_table[x].female);
		quest = "the art of thievery";
		SET_BIT(questmob->act, ACT_THIEF);
		break;

	case 4:
		for (maxnoun = 0; WarT_table[maxnoun].male != NULL; maxnoun++);

		x = number_range(0, --maxnoun);
		title = str_dup(questmob->sex == 1 ? WarT_table[x].male : WarT_table[x].female);
		quest = "the ways of weaponcraft";
		SET_BIT(questmob->act, ACT_WARRIOR);
		break;
	}

	sprintf(longdesc, "The %s, %s, stands here.\n", title, questmob->short_descr);
	questmob->long_descr = str_dup(longdesc);

	if ((questroom = generate_skillquest_room(ch, level)) == NULL) {
		bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
		return;
	}

	char_to_room(questmob, questroom);
	ch->pcdata->squestmob = questmob;

	if (type == 1) { /* mob quest */
		sprintf(buf, "Seek out the %s, %s, for teachings in %s!", title, questmob->short_descr, quest);
		do_say(questman, buf);
		sprintf(buf, "%s resides somewhere in the area of %s{x,", questmob->short_descr, questmob->in_room->area->name);
		do_say(questman, buf);
		sprintf(buf, "and can usually be found in %s{x.", questmob->in_room->name);
		do_say(questman, buf);
	}
	else if (type == 2) {   /* obj to mob quest */
		if ((questobj = generate_skillquest_obj(ch, level)) == NULL) {
			bug(" Bad generate_skillquest_obj, from generate_skillquest_mob", 0);
			return;
		}

		for (; ;) {
			if ((questroom = generate_skillquest_room(ch, level)) == NULL) {
				bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
				return;
			}

			if (questroom->area == questmob->in_room->area)
				continue;

			obj_to_room(questobj, questroom);
			break;
		}

		sprintf(buf, "The %s %s{x has reported missing the legendary artifact,",
		        title, questmob->short_descr);
		do_say(questman, buf);
		sprintf(buf, "the %s{x!  %s{x resides somewhere in %s{x,",
		        questobj->short_descr, questmob->short_descr, questmob->in_room->area->name);
		do_say(questman, buf);
		sprintf(buf, "and can usually be found in %s{x.", questmob->in_room->name);
		do_say(questman, buf);
		sprintf(buf, "%s last recalls travelling through %s{x, in the",
		        questmob->sex == 1 ? "He" : "She", questobj->in_room->name);
		do_say(questman, buf);
		sprintf(buf, "area of %s{x, when %s lost the treasure.",
		        questobj->in_room->area->name, questmob->sex == 1 ? "he" : "she");
		do_say(questman, buf);
	}
}

void generate_skillquest(CHAR_DATA *ch, CHAR_DATA *questman)
{
	OBJ_DATA *questobj;
	ROOM_INDEX_DATA *questroom;
	char buf[MAX_STRING_LENGTH];
	int level;
	level = ch->level;

	if (level > 91)
		level = 91;
	else if (level < 1)
		level = 1;

	ch->pcdata->sqcountdown = number_range(15, 30);
	ch->pcdata->squest_giver = questman->pIndexData->vnum;

	/* 40% chance of an item quest */
	if (chance(40)) {
		if ((questobj = generate_skillquest_obj(ch, level)) == NULL) {
			bug("Bad generate_skillquest_obj, from generate_skillquest", 0);
			return;
		}

		if ((questroom = generate_skillquest_room(ch, level)) == NULL) {
			bug("Bad generate_skillquest_room, from generate_skillquest", 0);
			return;
		}

		obj_to_room(questobj, questroom);
		ch->pcdata->squestloc1 = questroom->vnum;
		ch->pcdata->squestobj = questobj;
		sprintf(buf, "The legendary artifact, the %s{x, has been reported stolen!", questobj->short_descr);
		do_say(questman, buf);
		sprintf(buf, "The thieves were seen heading in the direction of %s{x,", questroom->area->name);
		do_say(questman, buf);
		do_say(questman, "and witnesses in that area say that they travelled");
		sprintf(buf, "through %s{x.", questroom->name);
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

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *room;
	OBJ_DATA *questitem;
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

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (!IS_NPC(victim)
			    || (victim->pIndexData == NULL
			        || victim->in_room == NULL
			        || victim->pIndexData->pShop != NULL)
			    || IS_SET(victim->imm_flags, IMM_SUMMON)
			    || IS_SET(victim->act, ACT_PET)
			    || !strcmp(victim->in_room->area->name, "Playpen")
			    || victim->in_room->clan
			    || IS_SET(victim->affected_by, AFF_CHARM)
			    || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE | ROOM_SOLITARY)
			    || IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_MALE_ONLY | ROOM_FEMALE_ONLY)
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
				victim = NULL;
				break; /* BREAK for no victims found */
			}
		}
	} /* end while */

	if (victim == NULL) {
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
			bug("Error creating quest item.", 0);
			return;
		}

		/* Add player's name to mox name to prevent visibility by others */
		sprintf(buf, "%s %s", questitem->name, ch->name);
		free_string(questitem->name);
		questitem->name = str_dup(buf);
		/* Mox timer added by Demonfire as a preventative measure against cheating.
		The countdown timer assignment was moved here so that it could be used
		in the mox timer calculation, it was normally assigned after the return
		of this function. */
		ch->countdown = number_range(10, 30);
		questitem->timer = (4 * ch->countdown + 10) / 3;
		obj_to_room(questitem, room);
		ch->questobj = questitem->pIndexData->vnum;
		sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr);
		do_say(questman, buf);
		do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
		sprintf(buf, "Look in the general area of %s for %s!", room->area->name, room->name);
		do_say(questman, buf);
		return;
	}
	else {
		/* Quest to kill a mob */
		switch (number_range(0, 3)) {
		case 0:
			sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",
			        victim->short_descr);
			do_say(questman, buf);
			do_say(questman, "This threat must be eliminated!");
			break;

		case 1:
			sprintf(buf, "Thera's most heinous criminal, %s, has escaped from the dungeon!",
			        victim->short_descr);
			do_say(questman, buf);
			sprintf(buf, "Since the escape, %s has murdered %d civilians!",
			        victim->short_descr, number_range(2, 20));
			do_say(questman, buf);
			do_say(questman, "The penalty for this crime is death, and you are to deliver the sentence!");
			break;

		case 2:
			sprintf(buf, "The Mayor of Midgaard has recently been attacked by %s.  This is an act of war!",
			        victim->short_descr);
			do_say(questman, buf);
			sprintf(buf, "%s must be severly dealt with for this injustice.",
			        victim->short_descr);
			do_say(questman, buf);
			break;

		case 3:
			sprintf(buf, "%s has been stealing valuables from the citizens of Solace.",
			        victim->short_descr);
			do_say(questman, buf);
			sprintf(buf, "Make sure that %s never has the chance to steal again.",
			        victim->short_descr);
			do_say(questman, buf);
			break;
		}

		if (room->name != NULL) {
			sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",
			        victim->short_descr, room->name);
			do_say(questman, buf);
			sprintf(buf, "That location is in the general area of %s.", room->area->name);
			do_say(questman, buf);
		}

		ch->countdown = number_range(10, 30);
		ch->questmob = victim->pIndexData->vnum;
	}
} /* end generate_quest() */

/* The main quest function */
void do_quest(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *questman;
	char buf [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		do_say(ch, "Don't be silly, mobs can't quest!");
		return;
	}

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0') {
		quest_usage(ch);
		return;
	}

	/* The following subfunctions are grouped in alphabetical order so I can find them.
	   Each is an independent IF block. Imm commands come after mortal commands starting
	   with the same letter so that imms do not execute the more powerful commands by
	   accident when they abbreviate too much. */

	/*** AWARD ***/
	if (IS_IMP(ch) && !str_prefix1(arg1, "award")) {
		char player[MAX_INPUT_LENGTH];
		int number = -1;
		CHAR_DATA *wch;
		DESCRIPTOR_DATA *d;
		argument = one_argument(argument, player);

		if (player[0] == '\0') {
			stc("Quest award to whom?\n", ch);
			return;
		}

		wch = get_player_world(ch, player, VIS_PLR);

		if (wch == NULL && str_prefix1("allchars", player)) {
			ptc(ch, "You find no player named '%s' in the game!\n", player);
			return;
		}

		if (argument != NULL && *argument != '\0')
			number = atoi(argument);

		if (number < 0 || number > 10) {
			ptc(ch, "Award %s how many quest points (0..10) ?\n", wch->name);
			return;
		}

		if (!str_prefix1("allchars", player)) {
			for (d = descriptor_list; d; d = d->next) {
				if (IS_PLAYING(d) && !IS_IMMORTAL(d->character)) {
					d->character->questpoints += number;
					ptc(ch, "You award %s %d questpoints.\n", d->character->name, number);
					ptc(d->character, "%s has awarded you %d questpoints.\n", ch->name, number);
				}
			}

			sprintf(buf, "Log %s: QUEST AWARD allchars %d", ch->name, number);
			wiznet(buf, ch, NULL, WIZ_SECURE, 0, GET_RANK(ch));
			log_string(buf);
			return;
		}

		wch->questpoints += number;
		ptc(ch, "You award %d quest points to %s.\n", number, wch->name);
		ptc(wch, "%s awards %d quest points to you.\n", ch->name, number);
		sprintf(buf, "Log %s: QUEST AWARD %d to %s", ch->name, number, player);
		wiznet(buf, ch, NULL, WIZ_SECURE, 0, GET_RANK(ch));
		log_string(buf);
		return;
	}

	/*** COMPLETE ***/
	if (!str_prefix1(arg1, "complete")) {
		OBJ_DATA *obj = NULL;
		int pointreward = 0;
		int reward = 0;
		int pracreward = 0;
		int sn = 0;

		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if ((questman = find_squestmaster(ch)) != NULL) {
			act("$n informs $N $e has completed $s skill quest.", ch, NULL, questman, TO_ROOM);
			act("You inform $N you have completed your skill quest.", ch, NULL, questman, TO_CHAR);

			if (ch->pcdata->squest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a skill quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
				do_say(questman, "But you aren't on a skill quest at the moment!");
				sprintf(buf, "You have to REQUEST a skill quest first, %s.", ch->name);
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
				for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
					if (obj->pIndexData == NULL) {
						bug("SQUEST COMPLETE: obj with NULL pIndexData", 0);
						obj = NULL;
						break;
					}
					else if (obj == ch->pcdata->squestobj)
						break;
				}

				/* check to see if they dropped it on the way */
				if (ch->pcdata->squestobjf && obj == NULL) {
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
					sprintf(buf, "You haven't spoken with %s yet!  Come back when you have.",
					        ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}
			}
			/* if they're looking for both a mob and an object */
			else if (ch->pcdata->squestmob && ch->pcdata->squestobj) {
				if (ch->pcdata->squestobjf && !ch->pcdata->squestmobf) {
					sprintf(buf, "You must return the %s to %s before you return here.",
					        ch->pcdata->squestobj->short_descr, ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}

				if (ch->pcdata->squestmobf && !ch->pcdata->squestobjf) { /* shouldn't happen */
					bug("SQuest mob found without object", 0);
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
				bug("Skill quest not found", 0);
				sq_cleanup(ch);
				return;
			}

			reward = number_range(ch->level / 4, ch->level * 3 / 2) + 1;
			pointreward = number_range(1, 10);
			/* Awards ceremony */
			do_say(questman, "Congratulations on completing your skill quest!");
			sprintf(buf, "As a reward, I am giving you %d skill point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), reward);
			do_say(questman, buf);
			sn = get_random_skill(ch);

			if (chance(20) && sn != -1) {
				sprintf(buf, "I will also teach you some of the finer points of %s.", skill_table[sn].name);
				do_say(questman, buf);
				ptc(ch, "%s helps you practice %s.\n", questman->short_descr, skill_table[sn].name);
				check_improve(ch, sn, TRUE, -1); /* always improve */
			}

			sq_cleanup(ch);
			ch->pcdata->nextsquest = pointreward * 2;
			ch->gold += reward;
			ch->pcdata->skillpoints += pointreward;
			wiznet("{Y:SKILL QUEST:{x $N has completed a skill quest", ch, NULL, WIZ_QUEST, 0, 0);
			return;
		}
		else if ((questman = find_questmaster(ch)) != NULL) {
			act("$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
			act("You inform $N you have completed your quest.", ch, NULL, questman, TO_CHAR);

			if (ch->quest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!IS_SET(ch->act, PLR_QUESTOR)) {
				do_say(questman, "But you aren't on a quest at the moment!");
				sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
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
				for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
					if (obj->pIndexData == NULL) {
						bug("QUEST COMPLETE: obj with NULL pIndexData", 0);
						obj = NULL;
						break;
					}
					else if (obj->pIndexData->vnum == ch->questobj)
						break;
				}
			}

			/* Added this in cause they drop their quest item after finding it */
			if (ch->questobf < 0 && obj == NULL) {
				do_say(questman, "You must have lost your quest item on the way here.  Hurry and find it!");
				ch->questobf = 0;
				return;
			}

			/* check for incomplete quest */
			if (ch->questmob > 0 || (ch->questobj > 0 && obj == NULL)) {
				do_say(questman, "You haven't completed the quest yet, but there is still time!");
				return;
			}

			/* rewards are calculated in the separate cases for mob and obj, but the ceremony is left
			   until after the endif to avoid duplicate code -- Elrac */
			if (ch->questobj > 0 && obj != NULL) {
				act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
				act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);
				extract_obj(obj);
				obj = NULL;
			}
			else if (ch->questmob != -1) {
				/* we shouldn't be here. */
				bug("QUEST COMPLETE: at reward phase without kill or object", 0);
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
			sprintf(buf, "As a reward, I am giving you %d quest point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), reward);
			do_say(questman, buf);

			if (pracreward > 0)
				ptc(ch, "{YYou also gain %d practice%s!{x\n", pracreward, (pracreward == 1 ? "" : "s"));

			REMOVE_BIT(ch->act, PLR_QUESTOR);
			ch->quest_giver = 0;
			ch->countdown = 0;
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;

			if (quest_double)
				ch->nextquest = pointreward;
			else
				ch->nextquest = pointreward * 2;

			ch->gold += reward;
			ch->questpoints += pointreward;
			ch->practice += pracreward;
			wiznet("{Y:QUEST:{x $N has completed a quest", ch, NULL, WIZ_QUEST, 0, 0);
			return;
		}
		else
			stc("You can't do that here.\n", ch);

		return;
	}

	/*** CLOSE ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "close")) {
		char num_arg[MAX_INPUT_LENGTH];
		int num_in_area;
		int num_to_oust = 0;
		CHAR_DATA *victim;
		ROOM_INDEX_DATA *temple;

		if (!quest_open) {
			stc("The quest area is not currently open.\n", ch);
			return;
		}

		argument = one_argument(argument, num_arg);

		if (num_arg[0] != '\0' && is_number(num_arg))
			num_to_oust = atoi(num_arg);

		num_in_area = quest_area->nplayer;

		if (num_in_area > 0) {
			if (num_to_oust < num_in_area - 1 || num_to_oust > num_in_area + 1) {
				ptc(ch, "There are still %d players in the quest area.\n", num_in_area);
				stc("Close the quest on how many players?\n", ch);
				return;
			}

			temple = get_room_index(ROOM_VNUM_TEMPLE);

			if (temple == NULL)
				bug("QUEST CLOSE: Temple location not found (%d)", ROOM_VNUM_TEMPLE);
			else {
				for (victim = char_list; victim != NULL; victim = victim->next) {
					if (!IS_NPC(victim) && victim->in_room != NULL
					    && victim->in_room->area == quest_area) {
						act("You expel $N from the quest area.", ch, NULL, victim, TO_CHAR);
						stc("You are expelled from the quest area.\n", victim);
						char_from_room(victim);
						char_to_room(victim, temple);
						act("$n arrives from a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
						do_look(victim, "auto");
					}
				}
			}
		}

		stc("*** You have closed the quest area ***\n", ch);
		sprintf(buf, "%s has closed the quest area.\n", ch->name);
		do_send_announce(ch, buf);
		quest_open = FALSE;
		return;
	}

	/*** DEDUCT ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "deduct")) {
		char arg[MAX_INPUT_LENGTH];
		CHAR_DATA *victim;
		int qpoint;
		argument = one_argument(argument, arg);

		if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
			stc("Player not found.\n", ch);
			return;
		}

		if (!is_number(argument)) {
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
		sprintf(buf, "%d questpoints have been deducted, %d remaining.\n", qpoint, victim->questpoints);
		stc(buf, ch);
		stc(buf, victim);
		sprintf(buf, "%s deducted %d qp from %s [%d remaining].", ch->name, qpoint, victim->name, victim->questpoints);
		wiznet(buf, ch, NULL, WIZ_SECURE, WIZ_QUEST, GET_RANK(ch));
		return;
	}

	/*** DOUBLE ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "double")) {
		if (!quest_double) {
			stc("You declare double QP for all!\n", ch);
			wiznet("{Y:QUEST:{x $N has declared double QP", ch, NULL, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared Double QP for all!  Happy Questing!");
			quest_double = 1;
		}
		else {
			stc("You declare normal QP for all.\n", ch);
			wiznet("{Y:QUEST:{x $N has declared normal QP", ch, NULL, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared normal questing.");
			quest_double = 0;
		}

		return;
	}

	/*** FORFEIT ***/
	if (!str_prefix1(arg1, "forfeit")) {
		if (!IS_SET(ch->act, PLR_QUESTOR) && !IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
			stc("You aren't currently on a quest.\n", ch);
			return;
		}

		if (IS_SET(ch->pcdata->plr, PLR_SQUESTOR) && find_squestmaster(ch) != NULL) {
			sq_cleanup(ch);
			ch->pcdata->nextsquest = 20;
			wiznet("{Y:SKILL QUEST:{x $N has forfeited $S skill quest", ch, NULL, WIZ_QUEST, 0, 0);
			stc("You have forfeited your skill quest.\n", ch);
			return;
		}

		if (IS_SET(ch->act, PLR_QUESTOR) && find_questmaster(ch) != NULL) {
			REMOVE_BIT(ch->act, PLR_QUESTOR);
			ch->quest_giver = 0;
			ch->countdown = 0;
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;
			ch->nextquest = 12;
			wiznet("{Y:QUEST:{x $N has forfeited $S quest", ch, NULL, WIZ_QUEST, 0, 0);
			stc("You have forfeited your quest.\n", ch);
			return;
		}

		stc("You can't do that here.\n", ch);
		return;
	}

	/*** INFO ***/
	if (!str_prefix1(arg1, "info")) {
		if (ch->in_room == NULL || ch->in_room->area == NULL) {
			stc("You cannot recall your quest from this location.\n", ch);
			return;
		}

		squest_info(ch);
		stc("\n", ch);
		quest_info(ch);
		return;
	}

	/*** JOIN ***/
	if (!str_prefix1(arg1, "join")) {
		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if (ch->in_room == NULL) {
			stc("Something prevents you from joining the quest.\n", ch);
			return;
		}

		if (!quest_open) {
			stc("Sorry, but the quest area has not been opened!\n", ch);
			return;
		}

		if (ch->level < quest_min || ch->level > quest_max) {
			stc("Sorry, but the quest area is not open to your level!\n", ch);
			return;
		}

		if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) {
			stc("You cannot join the quest from this location.\n", ch);
			return;
		}

		if (IS_AFFECTED(ch, AFF_CURSE)) {
			stc("You cannot join the quest in your current contition.\n", ch);
			return;
		}

		if (ch->fighting != NULL) {
			stc("You will need to stop fighting before you can join the quest.\n", ch);
			return;
		}

		act("$n joins the quest!", ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, quest_startroom);
		act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
		stc("You join the Quest!\n", ch);
		do_look(ch, "auto");
		return;
	}

	/*** LIST ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "list")) {
		char qblock[MAX_STRING_LENGTH], sqblock[MAX_STRING_LENGTH], mblock[MAX_STRING_LENGTH],
		     oblock[MAX_STRING_LENGTH], lblock[MAX_STRING_LENGTH];
		BUFFER *output;
		DESCRIPTOR_DATA *d;
		output = new_buf();
		stc("                            {YQuest                         {GSkill Quest{x\n", ch);
		stc("Name           Mobile Object Loctn   Time  QP   --- MobLoc ObjLoc  Time  SP\n", ch);
		stc("-----------------------------------------------------------------------------\n", ch);

		for (d = descriptor_list; d != NULL; d = d->next) {
			CHAR_DATA *wch;

			if (!IS_PLAYING(d) || !can_see_who(ch, d->character))
				continue;

			wch = (d->original != NULL) ? d->original : d->character;

			if (IS_NPC(wch) || !can_see_who(ch, wch))
				continue;

			if (IS_QUESTOR(wch)) {
				if (wch->questobj > 0) {                /* an item quest */
					sprintf(mblock, "{Y<%5d>{x", 0);

					if (wch->questobf < 0) {        /* item has been found */
						sprintf(oblock, "{C<{Yfound{C>{x");
						sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* item has not been found */
						sprintf(oblock, "{T<{Y%5d{T>{x", wch->questobj);
						sprintf(lblock, "{T<{Y%5d{T>{x", wch->questloc);
					}
				}
				else {                                  /* a mob quest */
					sprintf(oblock, "{Y<%5d>{x", 0);

					if (wch->questmob == -1) {      /* mob has been killed */
						sprintf(mblock, "{C<{Y dead{C>{x");
						sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* mob has not been killed */
						sprintf(mblock, "{T<{Y%5d{T>{x", wch->questmob);
						sprintf(lblock, "{T<{Y%5d{T>{x", wch->questloc);
					}
				}

				sprintf(qblock, "%s%s%s {Y[%2d][%4d]{x",
				        mblock, oblock, lblock, wch->countdown, wch->questpoints);
			}
			else
				sprintf(qblock, "<%5d><%5d><%5d> [%2d][%4d]",
				        0, 0, 0, wch->nextquest, wch->questpoints);

			if (IS_SQUESTOR(wch)) {
				if (wch->pcdata->squestmob == NULL      /* item quest */
				    && wch->pcdata->squestobj != NULL) {
					sprintf(mblock, "{G<%5d>{x", 0);

					if (wch->pcdata->squestobjf)    /* item has been found */
						sprintf(oblock, "{C<{Gfound{C>{x");
					else
						sprintf(oblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc1);
				}
				else if (wch->pcdata->squestobj == NULL /* mob quest */
				         && wch->pcdata->squestmob != NULL) {
					sprintf(oblock, "{G<%5d>{x", 0);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						sprintf(mblock, "{C<{Gfound{C>{x");
					else
						sprintf(mblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc2);
				}
				else if (wch->pcdata->squestobj != NULL /* item to mob quest */
				         && wch->pcdata->squestmob != NULL) {
					if (wch->pcdata->squestobjf)    /* item has been found */
						sprintf(oblock, "{C<{Gfound{C>{x");
					else
						sprintf(oblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc1);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						sprintf(mblock, "{C<{Gfound{C>{x");
					else
						sprintf(mblock, "{T<{G%5d{T>{x", wch->pcdata->squestloc2);
				}
				else {                                  /* shouldn't be here */
					sprintf(oblock, "{P<? ? ? ? ?>{x");
					sprintf(mblock, "{P<? ? ? ? ?>{x");
				}

				sprintf(sqblock, "%s%s {G[%2d][%4d]{x",
				        mblock, oblock, wch->pcdata->sqcountdown, wch->pcdata->skillpoints);
			}
			else
				sprintf(sqblock, "<%5d><%5d> [%2d][%4d]",
				        0, 0, wch->pcdata->nextsquest, wch->pcdata->skillpoints);

			sprintf(buf, "%-14s %s     %s{x\n", wch->name, qblock, sqblock);
			add_buf(output, buf);
		}

		page_to_char(buf_string(output), ch);
		free_buf(output);
		return;
	}

	/*** OPEN ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "open")) {
		char num_arg[MAX_INPUT_LENGTH];

		if ((quest_startroom = get_room_index(QUEST_STARTROOM)) == NULL) {
			stc("The quest area is not available in this reboot.\n", ch);
			return;
		}

		quest_area = quest_startroom->area;

		if (quest_open) {
			ptc(ch, "The quest area is already open, to levels %d to %d\n", quest_min, quest_max);
			return;
		}

		argument = one_argument(argument, num_arg);

		if (num_arg[0] != '\0' && is_number(num_arg))
			quest_min = atoi(num_arg);

		if (quest_min < 1 || quest_min > 100) {
			stc("Open the quest to which minimum level (1..100) ?\n", ch);
			return;
		}

		argument = one_argument(argument, num_arg);

		if (num_arg[0] != '\0' && is_number(num_arg))
			quest_max = atoi(num_arg);

		if (quest_max < quest_min || quest_max > 100) {
			ptc(ch, "Open the quest for levels %d to which maximum level (%d..100) ?\n",
			    quest_min, quest_min);
			return;
		}

		sprintf(buf, "%s has opened the quest area to levels %d through %d!\n", ch->name, quest_min, quest_max);
		do_send_announce(ch, buf);
		ptc(ch, "You open the quest area to levels %d through %d.\n", quest_min, quest_max);
		quest_open = TRUE;
		return;
	}

	/*** PK ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "pk")) {
		CHAR_DATA *salesgnome;
		ROOM_INDEX_DATA *to_room;
		salesgnome = get_mob_world(ch, "salesgnome", VIS_CHAR);

		if (salesgnome == NULL)
			bug("QUEST PK: salesgnome not there", 0);

		stc("PK in Questlands is now ", ch);
		quest_upk = !quest_upk;

		if (quest_upk) {
			stc("UNLIMITED (Questlands = Open Arena)\n", ch);
			wiznet("{Y:QUEST:{x $N has opened Questlands for Open Arena", ch, NULL, WIZ_QUEST, 0, 0);

			if (salesgnome != NULL) {
				to_room = get_room_index(ROOM_VNUM_ARENATICKET);

				if (to_room == NULL) {
					bug("QUEST PK: Can't find ticket booth", 0);
					return;
				}

				char_from_room(salesgnome);
				char_to_room(salesgnome, to_room);
			}
		}
		else {
			stc("LIMITED (like everywhere else)\n", ch);
			wiznet("{Y:QUEST:{x $N has restricted PK in Questlands", ch, NULL, WIZ_QUEST, 0, 0);

			if (salesgnome != NULL) {
				to_room = get_room_index(ROOM_VNUM_TICKETBACKROOM);

				if (to_room == NULL) {
					bug("QUEST PK: Can't find ticket booth back room", 0);
					return;
				}

				char_from_room(salesgnome);
				char_to_room(salesgnome, to_room);
			}
		}

		return;
	}

	/*** POINTS ***/
	if (!str_prefix1(arg1, "points")) {
		ptc(ch, "You have %d quest points and %d skill points.\n",
		    ch->questpoints, ch->pcdata->skillpoints);
		return;
	}

	/*** REQUEST ***/
	if (!str_prefix1(arg1, "request")) {
		if (get_position(ch) < POS_RESTING) {
			stc("You are too busy sleeping.\n", ch);
			return;
		}

		if ((questman = find_squestmaster(ch)) != NULL) {
			act("$n asks $N for a skill quest.", ch, NULL, questman, TO_ROOM);
			act("You ask $N for a skill quest.", ch, NULL, questman, TO_CHAR);

			if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET)) {
				if (ch->name != NULL)
					check_social(questman, "rofl", ch->name);

				sprintf(buf, "Who ever heard of a pet questing for its %s?",
				        ch->sex == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != NULL && ch->leader->name != NULL)
					check_social(questman, "laugh", ch->leader->name);

				return;
			}

			if (IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->pcdata->nextsquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'",
				    ch, NULL, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'",
				    ch, NULL, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, NULL, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, NULL, questman, TO_CHAR);
			sq_cleanup(ch);
			generate_skillquest(ch, questman);
			SET_BIT(ch->pcdata->plr, PLR_SQUESTOR);
			sprintf(buf, "You have %d minutes to complete this quest.", ch->pcdata->sqcountdown);
			do_say(questman, buf);
			do_say(questman, "May the gods go with you!");
			wiznet("{Y:SKILL QUEST:{x $N has begun a skill quest", ch, NULL, WIZ_QUEST, 0, 0);
			return;
		}

		if ((questman = find_questmaster(ch)) != NULL) {
			act("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
			act("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);

			if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET)) {
				if (ch->name != NULL)
					check_social(questman, "rofl", ch->name);

				sprintf(buf, "Who ever heard of a pet questing for its %s?",
				        ch->sex == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != NULL && ch->leader->name != NULL)
					check_social(questman, "laugh", ch->leader->name);

				return;
			}

			if (IS_SET(ch->act, PLR_QUESTOR)) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->nextquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, NULL, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, NULL, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, NULL, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, NULL, questman, TO_CHAR);
			ch->questmob = 0;
			ch->questobj = 0;
			ch->questobf = 0;
			ch->questloc = 0;
			generate_quest(ch, questman);

			if (ch->questmob > 0 || ch->questobj > 0) {
				SET_BIT(ch->act, PLR_QUESTOR);
				sprintf(buf, "You have %d minutes to complete this quest.", ch->countdown);
				do_say(questman, buf);
				do_say(questman, "May the gods go with you!");
			}

			wiznet("{Y:QUEST:{x $N has begun a quest", ch, NULL, WIZ_QUEST, 0, 0);
			return;
		}
		else
			stc("You can't do that here.\n", ch);

		return;
	}

	/*** STATUS ***/
	if (IS_IMMORTAL(ch) && !str_prefix1(arg1, "status")) {
		if (quest_open)
			stc("The Quest Area is currently open for a {Rquest{x.\n", ch);
		else
			stc("There is currently {Gno quest{x going on.\n", ch);

		if (quest_upk)
			stc("There is {Gunrestricted carnage{x in the Quest Area.\n", ch);
		else
			stc("The Quest Area is currently PK {Rrestricted{x.\n", ch);

		return;
	}

	/*** TIME ***/
	if (!str_prefix1(arg1, "time")) {
		if (!IS_SET(ch->act, PLR_QUESTOR)) {
			stc("You aren't currently on a quest.\n", ch);

			if (ch->nextquest > 1)
				ptc(ch, "There are %d minutes remaining until you can quest again.\n",
				    ch->nextquest);
			else if (ch->nextquest == 1)
				stc("There is less than a minute remaining until you can quest again.\n", ch);
		}
		else if (ch->countdown > 0)
			ptc(ch, "You have %d minutes left to complete your current quest.\n", ch->countdown);

		if (!IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
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

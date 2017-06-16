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

#include "Game.hh"
#include "Area.hh"
#include "find.hh"
#include "channels.hh"
#include "merc.hh"
#include "interp.hh"
#include "tables.hh"
#include "recycle.hh"
#include "Affect.hh"
#include "Format.hh"
#include "Quest.hh"

/* syllable tables */
#define MAXMSYL1        77
#define MAXMSYL2        38
#define MAXMSYL3        83
#define MAXFSYL1        76
#define MAXFSYL2        42
#define MAXFSYL3        54

void sq_cleanup(Character *ch)
{
	Quest *quest = ch->pcdata->quests.squest;

	if (!quest)
		return;

	Object *obj, *next_obj;
	for (obj = object_list; obj != nullptr; obj = next_obj) {
		next_obj = obj->next;

		if (quest->is_target(obj))
			extract_obj(obj);
	}

	Character *mob, *next_mob;
	for (mob = char_list; mob != nullptr ; mob = next_mob) {
		next_mob = mob->next;

		if (quest->is_target(mob))
			extract_char(mob, TRUE);
	}

	ch->pcdata->quests.stop_squest();
}


void squest_info(Character *ch)
{
	Quest *quest = ch->pcdata->quests.squest;

	if (!quest) {
		stc("You aren't currently on a skill quest.\n", ch);
		return;
	}

	MobilePrototype *qm = get_mob_index(quest->quest_giver);
	String questman;

	if (qm)
		questman = qm->short_descr;
	else
		questman = "your quest master";

	Character *mob = nullptr;
	RoomPrototype *mob_loc = nullptr;
	bool mob_found = false;
	Object *obj = nullptr;
	RoomPrototype *obj_loc = nullptr;
	bool obj_found = false;

	for (QuestTarget& t: quest->targets) {
		if (t.type == QuestTarget::Mob) {
			mob = dynamic_cast<Character *>(t.target);
			mob_loc = get_room_index(t.location);
			mob_found = t.is_complete;
		}
		else if (t.type == QuestTarget::Obj) {
			obj = dynamic_cast<Object *>(t.target);
			obj_loc = get_room_index(t.location);
			obj_found = t.is_complete;
		}
	}

	if (obj == nullptr && mob == nullptr) { /* no quest */
		stc("You've forgotten what your skill quest was.\n", ch);
		bug("QUEST INFO: skill quest with no obj or mob", 0);
		sq_cleanup(ch);
		return;
	}

	ptc(ch, "You recall the skill quest which %s gave you.\n", questman);

	if (obj != nullptr && mob == nullptr) { /* obj, no mob */
		if (obj_found) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n", questman);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s!\n", obj->short_descr);
		ptc(ch, "The artifact was last known to be in %s{x,\n", obj_loc->name);
		ptc(ch, "in the area known as %s{x.\n", obj_loc->area->name);
		return;
	}

	if (obj == nullptr && mob != nullptr) { /* mob, no obj */
		if (mob_found) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n", questman);
			return;
		}

		ptc(ch, "You are on a quest to learn from the legendary %s!\n", mob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(mob) == SEX_MALE ? "He" : "She", mob_loc->name);
		ptc(ch, "in the area known as %s{x.\n", mob_loc->area->name);
		return;
	}

	if (obj != nullptr && mob != nullptr) { /* mob and obj */

		if (obj_found && mob_found) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n", questman);
			return;
		}

		if (!mob_found && obj_found) {
			ptc(ch, "You must return the %s to %s.\n", obj->short_descr, mob->short_descr);
			ptc(ch, "%s can usually be found in %s{x,\n",
			    GET_ATTR_SEX(mob) == SEX_MALE ? "He" : "She", mob_loc->name);
			ptc(ch, "in the area known as %s{x.\n", mob_loc->area->name);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s,\n", obj->short_descr);
		ptc(ch, "from %s{x, in the area known as %s{x,\n", obj_loc->name, obj_loc->area->name);
		ptc(ch, "and return it to it's rightful owner, %s.\n", mob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(mob) == 1 ? "He" : "She", mob_loc->name);
		ptc(ch, "in the area known as %s{x.\n", mob_loc->area->name);
	}
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
		bug("Memory error creating quest object.", 0);
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
		bug("Bad skillquest mob vnum, from generate_skillquest_mob", 0);
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
		bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
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
			bug(" Bad generate_skillquest_obj, from generate_skillquest_mob", 0);
			return;
		}

		for (; ;) {
			if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
				bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
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
			bug("Bad generate_skillquest_obj, from generate_skillquest", 0);
			return;
		}

		if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
			bug("Bad generate_skillquest_room, from generate_skillquest", 0);
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

	if (questman->fighting != nullptr) {
		stc("Wait until the fighting stops.\n", ch);
		return nullptr;
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

	if (questman->fighting != nullptr) {
		stc("Wait until the fighting stops.\n", ch);
		return nullptr;
	}

	return questman;
} /* end find_squestmaster */

/* Obtain additional location information about sought item/mob */
void quest_where(Character *ch, char *what)
{
	RoomPrototype *room = get_room_index(ch->pcdata->quests.quest->target.location);
	ptc(ch, "Rumor has it this %s was last seen in the area known as %s", what, room->area->name);

	if (room->name.empty())
		ptc(ch, ".\n");
	else
		ptc(ch, ",\nnear %s.\n", room->name);
} /* end quest_where */

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
			log_string(buf);
			return;
		}

		wch->questpoints += number;
		ptc(ch, "You award %d quest points to %s.\n", number, wch->name);
		ptc(wch, "%s awards %d quest points to you.\n", ch->name, number);
		Format::sprintf(buf, "Log %s: QUEST AWARD %d to %s", ch->name, number, player);
		wiznet(buf, ch, nullptr, WIZ_SECURE, 0, GET_RANK(ch));
		log_string(buf);
		return;
	}

	/*** COMPLETE ***/
	if (arg1.is_prefix_of("complete")) {
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

			if (!ch->pcdata->quests.squest) {
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
				Object *obj;

				for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
					if (obj->pIndexData == nullptr) {
						bug("SQUEST COMPLETE: obj with nullptr pIndexData", 0);
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
			
			if (QuestManager::doubleqp)
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

			Quest *quest = ch->pcdata->quests.quest;

			if (!quest) {
				do_say(questman, "But you aren't on a quest at the moment!");
				Format::sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
				do_say(questman, buf);
				return;
			}

			if (quest->quest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!quest->is_complete) {
				do_say(questman, "You haven't completed the quest yet, but there is still time!");
				return;
			}

			if (quest->target.type == Quest::Target::Obj) {
				Object *obj;

				/* check if player has the quest object */
				for (obj = ch->carrying; obj != nullptr; obj = obj->next_content)
					if (quest->is_target(obj))
						break;

				/* Added this in cause they drop their quest item after finding it */
				if (obj == nullptr) {
					do_say(questman, "You must have lost your quest item on the way here.  Hurry and find it!");
					quest->is_complete = false;
					return;
				}

				/* rewards are calculated in the separate cases for mob and obj, but the ceremony is left
				   until after the endif to avoid duplicate code -- Elrac */
				act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
				act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);
				extract_obj(obj);
			}

			reward = number_range(ch->level / 2, ch->level * 5 / 2) + 1; /* +1 is for the off chance of a level 0 questor */

			if (QuestManager::doubleqp)
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

			ch->pcdata->quests.stop_quest();

			if (QuestManager::doubleqp)
				ch->pcdata->quests.nextquest = UMAX(1, pointreward/2);
			else
				ch->pcdata->quests.nextquest = pointreward;

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
				bug("QUEST CLOSE: Temple location not found (%d)", ROOM_VNUM_TEMPLE);
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
		if (!QuestManager::doubleqp) {
			stc("You declare double QP for all!\n", ch);
			wiznet("{Y:QUEST:{x $N has declared double QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared Double QP for all!  Happy Questing!");
			QuestManager::doubleqp = 1;
		}
		else {
			stc("You declare normal QP for all.\n", ch);
			wiznet("{Y:QUEST:{x $N has declared normal QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared normal questing.");
			QuestManager::doubleqp = 0;
		}

		return;
	}

	/*** FORFEIT ***/
	if (arg1.is_prefix_of("forfeit")) {
		if (!ch->pcdata->quests.quest && !ch->pcdata->quests.squest) {
			stc("You aren't currently on a quest.\n", ch);
			return;
		}

		if (ch->pcdata->quests.squest && find_squestmaster(ch) != nullptr) {
			sq_cleanup(ch);
			ch->pcdata->nextsquest = 10;
			wiznet("{Y:SKILL QUEST:{x $N has forfeited $S skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your skill quest.\n", ch);
			return;
		}

		if (ch->pcdata->quests.quest && find_questmaster(ch) != nullptr) {
			ch->pcdata->quests.stop_quest();
			ch->pcdata->quests.nextquest = 6;
			wiznet("{Y:QUEST:{x $N has forfeited $S quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your quest.\n", ch);
			return;
		}

		stc("You can't do that here.\n", ch);
		return;
	}

	/*** INFO ***/
	if (arg1.is_prefix_of("info")) {

		squest_info(ch);
		stc("\n", ch);

		Quest *quest = ch->pcdata->quests.quest;

		if (!quest) {
			stc("You aren't currently on a quest.\n", ch);
			return;
		}

		MobilePrototype *questman = get_mob_index(quest->quest_giver);

		if (quest->is_complete) {
			stc("Your quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr.empty() ? "your quest master" : questman->short_descr));
			return;
		}

		stc("You recall the quest which the questmaster gave you.\n", ch);

		if (quest->target.type == Quest::Target::Obj) { /* questing for an object */
			ptc(ch, "You are on a quest to recover the fabled %s!\n",
				get_obj_index(quest->target.vnum)->name);
			quest_where(ch, "treasure");
		}
		else if (quest->target.type == Quest::Target::Mob) { /* questing for a mob */
			ptc(ch, "You are on a quest to slay the dreaded %s!\n",
				get_mob_index(quest->target.vnum)->short_descr);
			quest_where(ch, "fiend");
		}

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

			if (wch->pcdata && wch->pcdata->quests.quest) {
				Quest *quest = wch->pcdata->quests.quest;

				if (quest->target.type == Quest::Target::Obj) { /* an item quest */
					Format::sprintf(mblock, "{Y<%5d>{x", 0);

					if (quest->is_complete) {        /* item has been found */
						Format::sprintf(oblock, "{C<{Yfound{C>{x");
						Format::sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* item has not been found */
						Format::sprintf(oblock, "{T<{Y%5d{T>{x", quest->target.vnum);
						Format::sprintf(lblock, "{T<{Y%5d{T>{x", quest->target.location);
					}
				}
				else {                                  /* a mob quest */
					Format::sprintf(oblock, "{Y<%5d>{x", 0);

					if (quest->is_complete) {      /* mob has been killed */
						Format::sprintf(mblock, "{C<{Y dead{C>{x");
						Format::sprintf(lblock, "{C<{Yfound{C>{x");
					}
					else {                          /* mob has not been killed */
						Format::sprintf(mblock, "{T<{Y%5d{T>{x", quest->target.vnum);
						Format::sprintf(lblock, "{T<{Y%5d{T>{x", quest->target.location);
					}
				}

				Format::sprintf(qblock, "%s%s%s {Y[%2d][%4d]{x",
				        mblock, oblock, lblock, quest->countdown, wch->questpoints);
			}
			else
				Format::sprintf(qblock, "<%5d><%5d><%5d> [%2d][%4d]",
				        0, 0, 0, wch->pcdata->quests.nextquest, wch->questpoints);

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
		if (Game::world().quest.area == nullptr) {
			stc("The quest area is not available in this reboot.\n", ch);
			return;
		}

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
			bug("QUEST PK: salesgnome not there", 0);

		stc("PK in Questlands is now ", ch);
		Game::world().quest.pk = !Game::world().quest.pk;

		if (Game::world().quest.pk) {
			stc("UNLIMITED (Questlands = Open Arena)\n", ch);
			wiznet("{Y:QUEST:{x $N has opened Questlands for Open Arena", ch, nullptr, WIZ_QUEST, 0, 0);

			if (salesgnome != nullptr) {
				to_room = get_room_index(ROOM_VNUM_ARENATICKET);

				if (to_room == nullptr) {
					bug("QUEST PK: Can't find ticket booth", 0);
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

			if (ch->pcdata->quests.squest) {
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

			if (ch->pcdata->quests.quest) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->pcdata->quests.nextquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_CHAR);

			ch->pcdata->quests.quest = Quest::generate(ch, questman);

			Format::sprintf(buf, "You have %d minutes to complete this quest.", ch->pcdata->quests.quest->countdown);
			do_say(questman, buf);
			do_say(questman, "May the gods go with you!");

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
		if (!ch->pcdata->quests.quest) {
			stc("You aren't currently on a quest.\n", ch);

			if (ch->pcdata->quests.nextquest > 1)
				ptc(ch, "There are %d minutes remaining until you can quest again.\n",
				    ch->pcdata->quests.nextquest);
			else if (ch->pcdata->quests.nextquest == 1)
				stc("There is less than a minute remaining until you can quest again.\n", ch);
		}
		else if (ch->pcdata->quests.quest->countdown > 0)
			ptc(ch, "You have %d minutes left to complete your current quest.\n", ch->pcdata->quests.quest->countdown);

		if (!ch->pcdata->quests.squest) {
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

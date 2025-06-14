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
#include "affect/Affect.hh"
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
#include "lootv2.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "Room.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "tables.hh"
#include "World.hh"
#include "RoomPrototype.hh"

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

void quest_cleanup(Character *ch) {
	ch->pcdata->plr_flags -= PLR_QUESTOR;
	ch->pcdata->quest_giver = 0;
	ch->pcdata->countdown = 0;
	ch->pcdata->questmob = 0;
	ch->pcdata->questobj = 0;
	ch->pcdata->questobf = 0;
	ch->pcdata->questloc = Location();
}

void sq_cleanup(Character *ch)
{
	Object *obj;
	ch->pcdata->plr_flags -= PLR_SQUESTOR;
	ch->pcdata->squest_giver = 0;
	ch->pcdata->sqcountdown = 0;

	if (ch->pcdata->squestobj != nullptr) {
		for (obj = Game::world().object_list; obj != nullptr; obj = obj->next)
			if (obj == ch->pcdata->squestobj) {
				extract_obj(obj);
				break;
			}

		ch->pcdata->squestobj = nullptr;
	}

	if (ch->pcdata->squestmob != nullptr) {
		if (!ch->pcdata->squestmob->is_garbage())
			extract_char(ch->pcdata->squestmob, true);

		ch->pcdata->squestmob = nullptr;
	}

	ch->pcdata->squestobjf = false;
	ch->pcdata->squestmobf = false;
}

bool quest_level_diff(int clevel, int mlevel)
{
	if (clevel <= 5 && mlevel <= 5)                                    return true;
	else if (clevel >  5 && clevel <= 15 && mlevel >  5 && mlevel <= 15)    return true;
	else if (clevel > 15 && clevel <= 20 && mlevel > 15 && mlevel <= 20)    return true;
	else if (clevel > 20 && clevel <= 25 && mlevel > 20 && mlevel <= 25)    return true;
	else if (clevel > 25 && clevel <= 35 && mlevel > 25 && mlevel <= 35)    return true;
	else if (clevel > 35 && clevel <= 45 && mlevel > 35 && mlevel <= 45)    return true;
	else if (clevel > 45 && clevel <= 50 && mlevel > 45 && mlevel <= 50)    return true;
	else if (clevel > 50 && clevel <= 55 && mlevel > 50 && mlevel <= 55)    return true;
	else if (clevel > 55 && clevel <= 65 && mlevel > 55 && mlevel <= 65)    return true;
	else if (clevel > 65 && clevel <= 75 && mlevel > 65 && mlevel <= 75)    return true;
	else if (clevel > 75 && clevel <= 85 && mlevel > 75 && mlevel <= 85)    return true;
	else if (clevel > 85 && mlevel >  85)                                   return true;
	else                                                                    return false;
}

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */
Character *find_questmaster(Character *ch)
{
	Character *questman;

	for (questman = ch->in_room->people; questman != nullptr; questman = questman->next_in_room) {
		if (!questman->is_npc())
			continue;

		if (questman->spec_fun == spec_lookup("spec_questmaster"))
			break;
	}

	if (questman == nullptr)
		return nullptr;

	if (questman->pIndexData == nullptr) {
		Logging::bug("find_questmaster: Questmaster has nullptr pIndexData!", 0);
		return nullptr;
	}

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
		if (!questman->is_npc())
			continue;

		if (questman->spec_fun == spec_lookup("spec_squestmaster"))
			break;
	}

	if (questman == nullptr)
		return nullptr;

	if (questman->pIndexData == nullptr) {
		Logging::bug("find_questmaster: Questmaster has nullptr pIndexData!", 0);
		return nullptr;
	}

	if (questman->fighting != nullptr) {
		stc("Wait until the fighting stops.\n", ch);
		return nullptr;
	}

	return questman;
} /* end find_squestmaster */

/* Obtain additional location information about sought item/mob */
void quest_where(Character *ch, char *what)
{
	Room *room;

	if (!ch->pcdata->questloc.is_valid()) {
		Logging::bugf("QUEST INFO: ch->pcdata->questloc = %d", ch->pcdata->questloc);
		return;
	}

	if (ch->in_room == nullptr)
		return;

	room = Game::world().get_room(ch->pcdata->questloc);

	ptc(ch, "Rumor has it this %s was last seen in the area known as %s", what, room->area().name);

	if (room->name().empty())
		ptc(ch, ".\n");
	else
		ptc(ch, ",\nnear %s.\n", room->name());
} /* end quest_where */

void squest_info(Character *ch)
{
	MobilePrototype *questman;
	Room *questroom_obj, *questroom_mob;

	if (!IS_SQUESTOR(ch)) {
		stc("You aren't currently on a skill quest.\n", ch);
		return;
	}

	if (ch->pcdata->squest_giver < 1) {
		Logging::bugf("QUEST INFO: quest giver = %d", ch->pcdata->squest_giver);
		stc("It seems the questmistress died of old age waiting for you.\n", ch);
		sq_cleanup(ch);
		return;
	}

	questman = Game::world().get_mob_prototype(ch->pcdata->squest_giver);

	if (questman == nullptr) {
		Logging::bugf("QUEST INFO: skill quest giver %d has no MobilePrototype!", ch->pcdata->squest_giver);
		stc("The questmistress has fallen very ill. Please contact an imm!\n", ch);
		sq_cleanup(ch);
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

		if ((questroom_obj = Game::world().get_room(ch->pcdata->squestloc1)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj quest with no location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s!\n",
		    ch->pcdata->squestobj->short_descr);
		ptc(ch, "The artifact was last known to be in %s{x,\n", questroom_obj->name());
		ptc(ch, "in the area known as %s{x.\n", questroom_obj->area().name);
		return;
	}

	if (ch->pcdata->squestobj == nullptr && ch->pcdata->squestmob != nullptr) { /* mob, no obj */
		if (ch->pcdata->squestmobf) {
			stc("Your skill quest is ALMOST complete!\n", ch);
			ptc(ch, "Get back to %s before your time runs out!\n",
			    (questman->short_descr.empty() ? "the questmistress" : questman->short_descr));
			return;
		}

		if ((questroom_mob = Game::world().get_room(ch->pcdata->squestloc2)) == nullptr) {
			Logging::bug("QUEST INFO: sqmob quest with no location", 0);
			stc("You've forgotten where your quest mob is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		ptc(ch, "You are on a quest to learn from the legendary %s!\n",
		    ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name());
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area().name);
		return;
	}

	if (ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob != nullptr) { /* mob and obj */
		if ((questroom_obj = Game::world().get_room(ch->pcdata->squestloc1)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj/mob quest with no obj location", 0);
			stc("You've forgotten where your quest object is.\n", ch);
			sq_cleanup(ch);
			return;
		}

		if ((questroom_mob = Game::world().get_room(ch->pcdata->squestloc2)) == nullptr) {
			Logging::bug("QUEST INFO: sqobj/mob quest with no mob location", 0);
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
			    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name());
			ptc(ch, "in the area known as %s{x.\n", questroom_mob->area().name);
			return;
		}

		ptc(ch, "You are on a quest to recover the legendary %s,\n", ch->pcdata->squestobj->short_descr);
		ptc(ch, "from %s{x, in the area known as %s{x,\n", questroom_obj->name(), questroom_obj->area().name);
		ptc(ch, "and return it to it's rightful owner, %s.\n", ch->pcdata->squestmob->short_descr);
		ptc(ch, "%s can usually be found in %s{x,\n",
		    GET_ATTR_SEX(ch->pcdata->squestmob) == 1 ? "He" : "She", questroom_mob->name());
		ptc(ch, "in the area known as %s{x.\n", questroom_mob->area().name);
	}
}

void quest_info(Character *ch)
{
	MobilePrototype *questman, *questinfo;
	ObjectPrototype *questinfoobj;

	if (!IS_QUESTOR(ch)) {
		stc("You aren't currently on a quest.\n", ch);
		return;
	}

	if (ch->pcdata->quest_giver < 1) {
		Logging::bugf("QUEST INFO: quest giver = %d", ch->pcdata->quest_giver);
		stc("It seems the questmaster died of old age waiting for you.\n", ch);
		quest_cleanup(ch);
		return;
	}

	questman = Game::world().get_mob_prototype(ch->pcdata->quest_giver);

	if (questman == nullptr) {
		Logging::bugf("QUEST INFO: quest giver %d has no MobilePrototype!", ch->pcdata->quest_giver);
		stc("The questmaster has fallen very ill. Please contact an imm!\n", ch);
		quest_cleanup(ch);
		return;
	}

	if (ch->pcdata->questmob == -1) { /* killed target mob */
		stc("Your quest is ALMOST complete!\n", ch);
		ptc(ch, "Get back to %s before your time runs out!\n",
		    (questman->short_descr.empty() ? "your quest master" : questman->short_descr));
		return;
	}
	else if (ch->pcdata->questobj > 0) { /* questing for an object */
		questinfoobj = Game::world().get_obj_prototype(ch->pcdata->questobj);

		if (questinfoobj != nullptr) {
			stc("You recall the quest which the questmaster gave you.\n", ch);
			ptc(ch, "You are on a quest to recover the fabled %s!\n", questinfoobj->name);
			quest_where(ch, "treasure");
			return;
		}

		/* quest object not found! */
		Logging::bugf("No info for quest object %d", ch->pcdata->questobj);
		quest_cleanup(ch);
		/* no RETURN -- fall thru to 'no quest', below */
	}
	else if (ch->pcdata->questmob > 0) { /* questing for a mob */
		questinfo = Game::world().get_mob_prototype(ch->pcdata->questmob);

		if (questinfo != nullptr) {
			ptc(ch, "You are on a quest to slay the dreaded %s!\n", questinfo->short_descr);
			quest_where(ch, "fiend");
			return;
		}

		/* quest mob not found! */
		Logging::bugf("No info for quest mob %d", ch->pcdata->questmob);
		quest_cleanup(ch);
		/* no RETURN -- fall thru to 'no quest', below */
	}

	/* we shouldn't be here */
	Logging::bug("QUEST INFO: Questor with no kill, mob or obj", 0);
	return;
}

skill::type get_random_skill(Character *ch)
{
	int count;
	skill::type sn;
	int pass = 1;
	int target = 0;

	/* pick a random skill or spell, if they have any */
	while (pass != 0) {
		count = 0;

		for (const auto& pair : skill_table) {
			sn = pair.first;

			if (get_skill_level(ch, sn) <= 0
			 || get_skill_level(ch, sn) >= 100)
				continue;

			const auto &entry = pair.second;

			if (entry.remort_guild != Guild::none && !CAN_USE_RSKILL(ch, sn))
				continue;

			if (get_usable_level(sn, ch->guild) > ch->level)
				continue;

			if (pass == 2 && count == target) {
				pass = 0;               /* get out of while loop */
				break;                  /* get out of for loop */
			}

			count++;
		}

		if (count == 0) { /* no skills or spells found */
			pass = 0;
			sn = skill::type::unknown;
		}

		if (pass != 0) {
			target = number_range(0, count-1);
			pass = 2;
		}
	}

	return sn;
}

void squestobj_to_squestmob(Character *ch, Object *obj, Character *mob)
{
	String buf;
	check_social(mob, "beam", ch->name);
	buf = Format::format("Thank you, %s, for returning my lost %s!", ch->name, obj->short_descr);
	do_say(mob, buf);
	extract_obj(obj);
	do_say(mob, "I left the gold reward with the Questmistress.  Farewell, and thank you!");
	act("$n turns and walks away.", mob, nullptr, nullptr, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	buf = Format::format("{Y:SKILL QUEST: {x$N has returned %s to %s", obj->short_descr, mob->short_descr);
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
	extract_char(mob, true);
	ch->pcdata->squestmobf = true;
}

void squestmob_found(Character *ch, Character *mob)
{
	if (ch->pcdata->squestmobf) {
		Logging::bug("At squestmob_found, player's squestmob already found.  Continuing...", 0);
		ch->pcdata->squestmobf = false;
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
	String buf = Format::format("{Y:SKILL QUEST: {x$N has spoken with %s", mob->short_descr);
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
	extract_char(mob, true);
	ch->pcdata->squestmobf = true;
}

Object *generate_skillquest_obj(Character *ch, int level)
{
	ExtraDescr *ed;
	Object *questobj;
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
	questobj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_SQUESTOBJ), level);

	if (! questobj) {
		Logging::bug("Memory error creating quest object.", 0);
		return nullptr;
	}

	questobj->timer = (4 * ch->pcdata->sqcountdown + 10) / 3;
	String buf = Format::format("%s %s", sq_item_table[descnum].name, ch->name);
	questobj->name        = buf;
	questobj->short_descr = sq_item_table[descnum].shortdesc;
	questobj->description = sq_item_table[descnum].longdesc;
	ed = new ExtraDescr(
		sq_item_table[descnum].name,
		sq_item_table[descnum].extended
	);
	ed->next           = questobj->extra_descr;
	questobj->extra_descr   = ed;
	ch->pcdata->squestobj = questobj;
	return questobj;
}

Room *generate_skillquest_room(Character *ch, int level)
{
	int pick = 0, pass = 1;

	while (pass <= 2) {
		int count = 0;

		for (auto& area_pair : Game::world().areas) {
			for (auto& entry : area_pair.second->rooms) {

				Room *room = entry.second;

				if (room == nullptr
				    || !can_see_room(ch, room)
				    || room->area().region != nullptr // no region areas for now
				    || room->area() == Game::world().quest.area()
				    || room->area().low_range > level
				    || room->area().high_range < level
				    || (room->area().min_vnum >= 24000      /* clanhall vnum ranges */
				        && room->area().min_vnum <= 26999)
				    || room->guild() != Guild::none
				    || room->area().name == "Playpen"
				    || room->area().name == "IMM-Zone"
				    || room->area().name == "Limbo"
				    || room->area().name == "Midgaard"
				    || room->area().name == "New Thalos"
				    || room->area().name == "Eilyndrae"     /* hack to make eilyndrae and torayna cri unquestable */
				    || room->area().name == "Torayna Cri"
				    || room->area().name == "The Abyss"
				    || room->area().name == "Battle Arenas"
				    || room->sector_type() == Sector::arena
				    || room->flags().has_any_of(
				              ROOM_MALE_ONLY
				              | ROOM_FEMALE_ONLY
				              | ROOM_PRIVATE
				              | ROOM_SOLITARY
				              | ROOM_NOQUEST))
					continue;

				/* no pet shops */
				Room *prev;
				if ((prev = Game::world().get_room(Location(Vnum(room->prototype.vnum.value() - 1)))) != nullptr)
					if (prev->flags().has(ROOM_PET_SHOP))
						continue;

				if (pass == 2 && count == pick)
					return room;

				count++;
			}
		}

		if (pass++ == 2 || count == 0)
			break;

		pick = number_range(0, count-1);
	}

	return nullptr;
}

void generate_skillquest_mob(Character *ch, Character *questman, int level, int type)
{
	Object  *questobj;
	Character *questmob;
	Room *questroom;
	int x;
	String title;
	String quest;
	String shortdesc;

	if ((questmob = create_mobile(Game::world().get_mob_prototype(MOB_VNUM_SQUESTMOB))) == nullptr) {
		Logging::bug("Bad skillquest mob vnum, from generate_skillquest_mob", 0);
		return;
	}

	ATTR_BASE(questmob, APPLY_SEX) = number_range(1, 2);
	questmob->level = ch->level;

	/* generate name */
	if (GET_ATTR_SEX(questmob) == 1)
		shortdesc = Format::format("%s%s%s",
		        Msyl1[number_range(0, (MAXMSYL1 - 1))],
		        Msyl2[number_range(0, (MAXMSYL2 - 1))],
		        Msyl3[number_range(0, (MAXMSYL3 - 1))]);
	else
		shortdesc = Format::format("%s%s%s",
		        Fsyl1[number_range(0, (MAXFSYL1 - 1))],
		        Fsyl2[number_range(0, (MAXFSYL2 - 1))],
		        Fsyl3[number_range(0, (MAXFSYL3 - 1))]);

	questmob->name = Format::format("squestmob %s", shortdesc);
	questmob->short_descr = shortdesc;

	/* generate title */
	switch (number_range(1, 4)) {
	case 1:
		x = number_range(0, MagT_table.size()-1);
		title = GET_ATTR_SEX(questmob) == 1 ? MagT_table[x].male : MagT_table[x].female;
		quest = "the powers of magic";
		questmob->act_flags += ACT_MAGE;
		break;

	case 2:
		x = number_range(0, CleT_table.size()-1);
		title = GET_ATTR_SEX(questmob) == 1 ? CleT_table[x].male : CleT_table[x].female;
		quest = "the wisdom of holiness";
		questmob->act_flags += ACT_CLERIC;
		break;

	case 3:
		x = number_range(0, ThiT_table.size()-1);
		title = GET_ATTR_SEX(questmob) == 1 ? ThiT_table[x].male : ThiT_table[x].female;
		quest = "the art of thievery";
		questmob->act_flags += ACT_THIEF;
		break;

	case 4:
		x = number_range(0, WarT_table.size()-1);
		title = GET_ATTR_SEX(questmob) == 1 ? WarT_table[x].male : WarT_table[x].female;
		quest = "the ways of weaponcraft";
		questmob->act_flags += ACT_WARRIOR;
		break;
	}

	questmob->long_descr = Format::format("The %s, %s, stands here.\n", title, questmob->short_descr);

	if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
		Logging::bug("Bad generate_skillquest_room, from generate_skillquest_mob", 0);
		return;
	}

	char_to_room(questmob, questroom);
	ch->pcdata->squestmob = questmob;

	if (type == 1) { /* mob quest */
		do_say(questman, Format::format("Seek out the %s, %s, for teachings in %s!", title, questmob->short_descr, quest));
		do_say(questman, Format::format("%s resides somewhere in the area of %s{x,", questmob->short_descr, questmob->in_room->area().name));
		do_say(questman, Format::format("and can usually be found in %s{x.", questmob->in_room->name()));
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

			if (questroom->area() == questmob->in_room->area())
				continue;

			obj_to_room(questobj, questroom);
			break;
		}

		do_say(questman, Format::format("The %s %s{x has reported missing the legendary artifact,",
		        title, questmob->short_descr));
		do_say(questman, Format::format("the %s{x!  %s{x resides somewhere in %s{x,",
		        questobj->short_descr, questmob->short_descr, questmob->in_room->area().name));
		do_say(questman, Format::format("and can usually be found in %s{x.", questmob->in_room->name()));
		do_say(questman, Format::format("%s last recalls travelling through %s{x, in the",
		        GET_ATTR_SEX(questmob) == 1 ? "He" : "She", questobj->in_room->name()));
		do_say(questman, Format::format("area of %s{x, when %s lost the treasure.",
		        questobj->in_room->area().name, GET_ATTR_SEX(questmob) == 1 ? "he" : "she"));
	}
}

void generate_skillquest(Character *ch, Character *questman)
{
	Object *questobj;
	Room *questroom;

	int level = URANGE(1, ch->level, LEVEL_HERO);
	ch->pcdata->sqcountdown = number_range(15, 30);
	ch->pcdata->squest_giver = questman->pIndexData->vnum;

	/* 40% chance of an item quest */
	if (roll_chance(40)) {
		if ((questobj = generate_skillquest_obj(ch, level)) == nullptr) {
			Logging::bug("Bad generate_skillquest_obj, from generate_skillquest", 0);
			return;
		}

		if ((questroom = generate_skillquest_room(ch, level)) == nullptr) {
			Logging::bug("Bad generate_skillquest_room, from generate_skillquest", 0);
			return;
		}

		obj_to_room(questobj, questroom);
		ch->pcdata->squestloc1 = questroom->location;
		ch->pcdata->squestobj = questobj;
		do_say(questman, Format::format("The legendary artifact, the %s{x, has been reported stolen!", questobj->short_descr));
		do_say(questman, Format::format("The thieves were seen heading in the direction of %s{x,", questroom->area().name));
		do_say(questman, "and witnesses in that area say that they travelled");
		do_say(questman, Format::format("through %s{x.", questroom->name()));
		return;
	}
	/* 40% chance of a mob quest */
	else if (roll_chance(66)) {
		generate_skillquest_mob(ch, questman, level, 1);
		ch->pcdata->squestloc2 = ch->pcdata->squestmob->in_room->location;
	}
	/* 20% chance of a obj to mob quest */
	else {
		generate_skillquest_mob(ch, questman, level, 2);
		ch->pcdata->squestloc1 = ch->pcdata->squestobj->in_room->location;
		ch->pcdata->squestloc2 = ch->pcdata->squestmob->in_room->location;
	}
}

void generate_quest(Character *ch, Character *questman)
{
	Character *victim;
	Room *room;
	Object *questitem;
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

		for (auto vch : Game::world().char_list) {
			if (!vch->is_npc()
			    || vch->pIndexData == nullptr
			    || vch->in_room == nullptr
			    || vch->pIndexData->pShop != nullptr
			    || vch->act_flags.has(ACT_NOSUMMON)
			    || vch->act_flags.has(ACT_PET)
			    || !strcmp(vch->in_room->area().name, "Playpen")
			    || vch->in_room->area().region != nullptr // no region areas for now
			    || vch->in_room->clan()
			    || affect::exists_on_char(vch, affect::type::charm_person)
			    || vch->in_room->flags().has_any_of(ROOM_PRIVATE | ROOM_SOLITARY)
			    || vch->in_room->flags().has_any_of(ROOM_SAFE | ROOM_MALE_ONLY | ROOM_FEMALE_ONLY)
			    || quest_level_diff(ch->level, vch->level) != true)
				continue;

			victim = vch;

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
		ch->pcdata->nextquest = 1;
		return;
	}

	/* at this point the player is sure to get a quest */
	room = victim->in_room;
	ch->pcdata->quest_giver = questman->pIndexData->vnum;
	ch->pcdata->questloc = room->location;

	/*  40% chance it will send the player on a 'recover item' quest. */
	if (roll_chance(40)) {
		int objvnum = 0;

		switch (number_range(0, 4)) {
		case 0: objvnum = QUEST_OBJQUEST1;      break;

		case 1: objvnum = QUEST_OBJQUEST2;      break;

		case 2: objvnum = QUEST_OBJQUEST3;      break;

		case 3: objvnum = QUEST_OBJQUEST4;      break;

		case 4: objvnum = QUEST_OBJQUEST5;      break;
		}

		questitem = create_object(Game::world().get_obj_prototype(objvnum), ch->level);

		if (! questitem) {
			Logging::bug("Error creating quest item.", 0);
			return;
		}

		/* Add player's name to mox name to prevent visibility by others */
		questitem->name = Format::format("%s %s", questitem->name, ch->name);
		/* Mox timer added by Demonfire as a preventative measure against cheating.
		The countdown timer assignment was moved here so that it could be used
		in the mox timer calculation, it was normally assigned after the return
		of this function. */
		ch->pcdata->countdown = number_range(10, 30);
		questitem->timer = (4 * ch->pcdata->countdown + 10) / 3;
		obj_to_room(questitem, room);
		ch->pcdata->questobj = questitem->pIndexData->vnum;
		do_say(questman, Format::format("Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr));
		do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
		do_say(questman, Format::format("Look in the general area of %s for %s!", room->area().name, room->name()));
		return;
	}
	else {
		/* Quest to kill a mob */
		switch (number_range(0, 3)) {
		case 0:
			do_say(questman, Format::format("An enemy of mine, %s, is making vile threats against the crown.",
			        victim->short_descr));
			do_say(questman, "This threat must be eliminated!");
			break;

		case 1:
			do_say(questman, Format::format("Thera's most heinous criminal, %s, has escaped from the dungeon!",
			        victim->short_descr));
			do_say(questman, Format::format("Since the escape, %s has murdered %d civilians!",
			        victim->short_descr, number_range(2, 20)));
			do_say(questman, "The penalty for this crime is death, and you are to deliver the sentence!");
			break;

		case 2:
			do_say(questman, Format::format("The Mayor of Midgaard has recently been attacked by %s.  This is an act of war!",
			        victim->short_descr));
			do_say(questman, Format::format("%s must be severly dealt with for this injustice.",
			        victim->short_descr));
			break;

		case 3:
			do_say(questman, Format::format("%s has been stealing valuables from the citizens of Solace.",
			        victim->short_descr));
			do_say(questman, Format::format("Make sure that %s never has the chance to steal again.",
			        victim->short_descr));
			break;
		}

		if (!room->name().empty()) {
			do_say(questman, Format::format("Seek %s out somewhere in the vicinity of %s!",
			        victim->short_descr, room->name()));
			do_say(questman, Format::format("That location is in the general area of %s.", room->area().name));
		}

		ch->pcdata->countdown = number_range(10, 30);
		ch->pcdata->questmob = victim->pIndexData->vnum;
	}
} /* end generate_quest() */

/* The main quest function */
void do_quest(Character *ch, String argument)
{
	Character *questman;
	String buf;

	if (ch->is_npc()) {
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

		if (player.empty()) {
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
				if (d->is_playing() && !IS_IMMORTAL(d->character) && !d->character->is_npc()) {
					d->character->pcdata->questpoints += number;
					ptc(ch, "You award %s %d questpoints.\n", d->character->name, number);
					ptc(d->character, "%s has awarded you %d questpoints.\n", ch->name, number);
				}
			}

			String buf = Format::format("Log %s: QUEST AWARD allchars %d", ch->name, number);
			wiznet(buf, ch, nullptr, WIZ_SECURE, 0, GET_RANK(ch));
			Logging::log(buf);
			return;
		}

		wch->pcdata->questpoints += number;
		ptc(ch, "You award %d quest points to %s.\n", number, wch->name);
		ptc(wch, "%s awards %d quest points to you.\n", ch->name, number);
		buf = Format::format("Log %s: QUEST AWARD %d to %s", ch->name, number, player);
		wiznet(buf, ch, nullptr, WIZ_SECURE, 0, GET_RANK(ch));
		Logging::log(buf);
		return;
	}

	/*** COMPLETE ***/
	if (arg1.is_prefix_of("complete")) {
		Object *obj = nullptr;

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

			if (!IS_SQUESTOR(ch)) {
				do_say(questman, "But you aren't on a skill quest at the moment!");
				buf = Format::format("You have to REQUEST a skill quest first, %s.", ch->name);
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
					ch->pcdata->squestobjf = false;
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
					buf = Format::format("You haven't spoken with %s yet!  Come back when you have.",
					        ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}
			}
			/* if they're looking for both a mob and an object */
			else if (ch->pcdata->squestmob && ch->pcdata->squestobj) {
				if (ch->pcdata->squestobjf && !ch->pcdata->squestmobf) {
					buf = Format::format("You must return the %s to %s before you return here.",
					        ch->pcdata->squestobj->short_descr, ch->pcdata->squestmob->short_descr);
					do_say(questman, buf);
					return;
				}

				if (ch->pcdata->squestmobf && !ch->pcdata->squestobjf) { /* shouldn't happen */
					Logging::bug("SQuest mob found without object", 0);
					do_say(questman, "They must like you a great deal...");
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

			int pointreward = number_range(1, 10);
			
			// set the next quest time here so it is based on the base award, not modifiers
			ch->pcdata->nextsquest = pointreward;

			if (Game::quest_double)
				pointreward += number_range(0, 10);

			/*suffix
			 *placeholder for skill quest suffixes
			 * of The Skilled 			+1 sp (2 if double on)
			 * of The Skillful 			+2 sp (4 if double on)
			 * 
			 replace section between above snips with the following:
			 */
			pointreward += number_range(0, GET_ATTR(ch, APPLY_SKILLPOINTS));
			int goldreward = number_range(ch->level / 4, ch->level * 3 / 2) + 1;
			
			/* Awards ceremony */
			do_say(questman, "Congratulations on completing your skill quest!");
			buf = Format::format("As a reward, I am giving you %d skill point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), goldreward);
			do_say(questman, buf);

	                if (!ch->revoke_flags.has(REVOKE_EXP)) {
				int xp = number_range(100, 300);
		                ptc(ch, "{PYou receive %d experience points.{x\n", xp);
        	                gain_exp(ch, xp);
			}

			skill::type sn = get_random_skill(ch);

			if (roll_chance(20) && sn != skill::type::unknown) {
				buf = Format::format("I will also teach you some of the finer points of %s.", skill::lookup(sn).name);
				do_say(questman, buf);
				ptc(ch, "%s helps you practice %s.\n", questman->short_descr, skill::lookup(sn).name);
				check_improve(ch, sn, true, -1); /* always improve */
			}

			sq_cleanup(ch);
			ch->gold += goldreward;
			ch->pcdata->skillpoints += pointreward;
			wiznet("{Y:SKILL QUEST:{x $N has completed a skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}
		else if ((questman = find_questmaster(ch)) != nullptr) {
			act("$n informs $N $e has completed $s quest.", ch, nullptr, questman, TO_ROOM);
			act("You inform $N you have completed your quest.", ch, nullptr, questman, TO_CHAR);

			if (ch->pcdata->quest_giver != questman->pIndexData->vnum) {
				do_say(questman, "I never sent you on a quest!  Perhaps you're thinking of someone else.");
				return;
			}

			if (!IS_QUESTOR(ch)) {
				do_say(questman, "But you aren't on a quest at the moment!");
				buf = Format::format("You have to REQUEST a quest first, %s.", ch->name);
				do_say(questman, buf);
				/* clean up, just in case */
				quest_cleanup(ch);
				return;
			}

			if (ch->pcdata->nextquest > 0 || ch->pcdata->countdown <= 0) {
				do_say(questman, "But you didn't complete your quest in time!");
				/* clean up, just in case */
				quest_cleanup(ch);
				return;
			}

			/* check if player has the quest object */
			if (ch->pcdata->questobj > 0) {
				for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
					if (obj->pIndexData == nullptr) {
						Logging::bug("QUEST COMPLETE: obj with nullptr pIndexData", 0);
						obj = nullptr;
						break;
					}
					else if (obj->pIndexData->vnum == ch->pcdata->questobj)
						break;
				}
			}

			/* Added this in cause they drop their quest item after finding it */
			if (ch->pcdata->questobf < 0 && obj == nullptr) {
				do_say(questman, "You must have lost your quest item on the way here.  Hurry and find it!");
				ch->pcdata->questobf = 0;
				return;
			}

			/* check for incomplete quest */
			if (ch->pcdata->questmob > 0 || (ch->pcdata->questobj > 0 && obj == nullptr)) {
				do_say(questman, "You haven't completed the quest yet, but there is still time!");
				return;
			}

			/* rewards are calculated in the separate cases for mob and obj, but the ceremony is left
			   until after the endif to avoid duplicate code -- Elrac */
			if (ch->pcdata->questobj > 0 && obj != nullptr) {
				act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
				act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);
				extract_obj(obj);
				obj = nullptr;
			}
			else if (ch->pcdata->questmob != -1) {
				/* we shouldn't be here. */
				Logging::bug("QUEST COMPLETE: at reward phase without kill or object", 0);
				return;
			}

			int pointreward = number_range(1, 5);

			// set the next quest time here so it is based on the base award, not modifiers
			ch->pcdata->nextquest = pointreward;

			if (Game::quest_double)
				pointreward += number_range(0, 5);

			/*suffix
			 *placeholder for skill quest suffixes
			 * of The Initiate Questor 			+1 qp (2 if double on)
			 * of The Adv. Questor 				+2 qp (4 if double on)
			 * of The Master Questor			+3 qp (6 if double on)
			 * 
			 replace section between above snips with the following:
			 */
			pointreward += number_range(0, GET_ATTR(ch, APPLY_QUESTPOINTS));

			int pracreward = 0;
			if (roll_chance(5))
				pracreward = number_range(1, 3);

			int goldreward = number_range(ch->level / 2, ch->level * 5 / 2) + 1; /* +1 is for the off chance of a level 0 questor */

			/* Awards ceremony */
			do_say(questman, "Congratulations on completing your quest!");
			buf = Format::format("As a reward, I am giving you %d quest point%s and %d gold.",
			        pointreward, (pointreward == 1 ? "" : "s"), goldreward);
			do_say(questman, buf);

	                if (!ch->revoke_flags.has(REVOKE_EXP)) {
				int xp = number_range(100, 300);
		                ptc(ch, "{PYou receive %d experience points.{x\n", xp);
        	                gain_exp(ch, xp);
			}

			if (pracreward > 0)
				ptc(ch, "{YYou also gain %d practice%s!{x\n", pracreward, (pracreward == 1 ? "" : "s"));

			quest_cleanup(ch);

			ch->gold += goldreward;
			ch->pcdata->questpoints += pointreward;
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
		Room *temple;

		if (!Game::world().quest.open) {
			stc("The quest area is not currently open.\n", ch);
			return;
		}

		String num_arg;
		argument = one_argument(argument, num_arg);

		if (!num_arg.empty() && num_arg.is_number())
			num_to_oust = atoi(num_arg);

		num_in_area = Game::world().quest.area().num_players();

		if (num_in_area > 0) {
			if (num_to_oust < num_in_area - 1 || num_to_oust > num_in_area + 1) {
				ptc(ch, "There are still %d players in the quest area.\n", num_in_area);
				stc("Close the quest on how many players?\n", ch);
				return;
			}

			temple = Game::world().get_room(Location(Vnum(ROOM_VNUM_TEMPLE)));

			if (temple == nullptr)
				Logging::bug("QUEST CLOSE: Temple location not found (%d)", ROOM_VNUM_TEMPLE);
			else {
				for (auto victim : Game::world().char_list) {
					if (!victim->is_npc() && victim->in_room != nullptr
					    && victim->in_room->area() == Game::world().quest.area()) {
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
		buf = Format::format("%s has closed the quest area.\n", ch->name);
		do_send_announce(ch, buf);
		Game::world().quest.open = false;
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

		if (qpoint > victim->pcdata->questpoints) {
			stc("They do not have enough quest points for that.\n", ch);
			return;
		}

		victim->pcdata->questpoints -= qpoint;
		buf = Format::format("%d questpoints have been deducted, %d remaining.\n", qpoint, victim->pcdata->questpoints);
		stc(buf, ch);
		stc(buf, victim);
		buf = Format::format("%s deducted %d qp from %s [%d remaining].", ch->name, qpoint, victim->name, victim->pcdata->questpoints);
		wiznet(buf, ch, nullptr, WIZ_SECURE, WIZ_QUEST, GET_RANK(ch));
		return;
	}

	/*** DOUBLE ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("double")) {
		if (!Game::quest_double) {
			stc("You declare double QP for all!\n", ch);
			wiznet("{Y:QUEST:{x $N has declared double QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared Double QP for all!  Happy Questing!");
			Game::quest_double = 1;
		}
		else {
			stc("You declare normal QP for all.\n", ch);
			wiznet("{Y:QUEST:{x $N has declared normal QP", ch, nullptr, WIZ_QUEST, 0, 0);
			do_send_announce(ch, "The Gods have declared normal questing.");
			Game::quest_double = 0;
		}

		return;
	}

	/*** FORFEIT ***/
	if (arg1.is_prefix_of("forfeit")) {
		if (!IS_QUESTOR(ch) && !IS_SQUESTOR(ch)) {
			stc("You aren't currently on a quest.\n", ch);
			return;
		}

		if (IS_SQUESTOR(ch) && find_squestmaster(ch) != nullptr) {
			sq_cleanup(ch);
			ch->pcdata->nextsquest = 10;
			wiznet("{Y:SKILL QUEST:{x $N has forfeited $S skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your skill quest.\n", ch);
			return;
		}

		if (IS_QUESTOR(ch) && find_questmaster(ch) != nullptr) {
			quest_cleanup(ch);
			ch->pcdata->nextquest = 6;
			wiznet("{Y:QUEST:{x $N has forfeited $S quest", ch, nullptr, WIZ_QUEST, 0, 0);
			stc("You have forfeited your quest.\n", ch);
			return;
		}

		stc("You can't do that here.\n", ch);
		return;
	}

	/*** INFO ***/
	if (arg1.is_prefix_of("info")) {
		if (ch->in_room == nullptr) {
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

		if (ch->in_room->flags().has(ROOM_NO_RECALL)) {
			stc("You cannot join the quest from this location.\n", ch);
			return;
		}

		if (affect::exists_on_char(ch, affect::type::curse)) {
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
		String qblock, sqblock, mblock, oblock, lblock;
		String output;
		Descriptor *d;
		stc("                            {YQuest                         {GSkill Quest{x\n", ch);
		stc("Name           Mobile Object Loctn   Time  QP   --- MobLoc ObjLoc  Time  SP\n", ch);
		stc("-----------------------------------------------------------------------------\n", ch);

		for (d = descriptor_list; d != nullptr; d = d->next) {
			Character *wch;

			if (!d->is_playing() || !can_see_who(ch, d->character))
				continue;

			wch = (d->original != nullptr) ? d->original : d->character;

			if (wch->is_npc() || !can_see_who(ch, wch))
				continue;

			if (IS_QUESTOR(wch)) {
				if (wch->pcdata->questobj > 0) {                /* an item quest */
					mblock = Format::format("{Y<%5d>{x", 0);

					if (wch->pcdata->questobf < 0) {        /* item has been found */
						oblock = Format::format("{C<{Yfound{C>{x");
						lblock = Format::format("{C<{Yfound{C>{x");
					}
					else {                          /* item has not been found */
						oblock = Format::format("{T<{Y%5d{T>{x", wch->pcdata->questobj);
						lblock = Format::format("{T<{Y%5s{T>{x", wch->pcdata->questloc);
					}
				}
				else {                                  /* a mob quest */
					oblock = Format::format("{Y<%5d>{x", 0);

					if (wch->pcdata->questmob == -1) {      /* mob has been killed */
						mblock = Format::format("{C<{Y dead{C>{x");
						lblock = Format::format("{C<{Yfound{C>{x");
					}
					else {                          /* mob has not been killed */
						mblock = Format::format("{T<{Y%5d{T>{x", wch->pcdata->questmob);
						lblock = Format::format("{T<{Y%5s{T>{x", wch->pcdata->questloc);
					}
				}

				qblock = Format::format("%s%s%s {Y[%2d][%4d]{x",
				        mblock, oblock, lblock, wch->pcdata->countdown, wch->pcdata->questpoints);
			}
			else
				qblock = Format::format("<%5d><%5d><%5d> [%2d][%4d]",
				        0, 0, 0, wch->pcdata->nextquest, wch->pcdata->questpoints);

			if (IS_SQUESTOR(wch)) {
				if (wch->pcdata->squestmob == nullptr      /* item quest */
				    && wch->pcdata->squestobj != nullptr) {
					mblock = Format::format("{G<%5d>{x", 0);

					if (wch->pcdata->squestobjf)    /* item has been found */
						oblock = Format::format("{C<{Gfound{C>{x");
					else
						oblock = Format::format("{T<{G%5s{T>{x", wch->pcdata->squestloc1);
				}
				else if (wch->pcdata->squestobj == nullptr /* mob quest */
				         && wch->pcdata->squestmob != nullptr) {
					oblock = Format::format("{G<%5d>{x", 0);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						mblock = Format::format("{C<{Gfound{C>{x");
					else
						mblock = Format::format("{T<{G%5s{T>{x", wch->pcdata->squestloc2);
				}
				else if (wch->pcdata->squestobj != nullptr /* item to mob quest */
				         && wch->pcdata->squestmob != nullptr) {
					if (wch->pcdata->squestobjf)    /* item has been found */
						oblock = Format::format("{C<{Gfound{C>{x");
					else
						oblock = Format::format("{T<{G%5s{T>{x", wch->pcdata->squestloc1);

					if (wch->pcdata->squestmobf)    /* mob has been found */
						mblock = Format::format("{C<{Gfound{C>{x");
					else
						mblock = Format::format("{T<{G%5s{T>{x", wch->pcdata->squestloc2);
				}
				else {                                  /* shouldn't be here */
					oblock = Format::format("{P<? ? ? ? ?>{x");
					mblock = Format::format("{P<? ? ? ? ?>{x");
				}

				sqblock = Format::format("%s%s {G[%2d][%4d]{x",
				        mblock, oblock, wch->pcdata->sqcountdown, wch->pcdata->skillpoints);
			}
			else
				sqblock = Format::format("<%5d><%5d> [%2d][%4d]",
				        0, 0, wch->pcdata->nextsquest, wch->pcdata->skillpoints);

			output += Format::format("%-14s %s     %s{x\n", wch->name, qblock, sqblock);
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

		buf = Format::format("%s has opened the quest area to levels %d through %d!\n", ch->name, Game::world().quest.min_level, Game::world().quest.max_level);
		do_send_announce(ch, buf);
		ptc(ch, "You open the quest area to levels %d through %d.\n", Game::world().quest.min_level, Game::world().quest.max_level);
		Game::world().quest.open = true;
		return;
	}

	/*** PK ***/
	if (IS_IMMORTAL(ch) && arg1.is_prefix_of("pk")) {
		Character *salesgnome;
		Room *to_room;
		salesgnome = get_mob_world(ch, "salesgnome", VIS_CHAR);

		if (salesgnome == nullptr)
			Logging::bug("QUEST PK: salesgnome not there", 0);

		stc("PK in Questlands is now ", ch);
		Game::world().quest.pk = !Game::world().quest.pk;

		if (Game::world().quest.pk) {
			stc("UNLIMITED (Questlands = Open Arena)\n", ch);
			wiznet("{Y:QUEST:{x $N has opened Questlands for Open Arena", ch, nullptr, WIZ_QUEST, 0, 0);

			if (salesgnome != nullptr) {
				to_room = Game::world().get_room(Location(Vnum(ROOM_VNUM_ARENATICKET)));

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
				to_room = Game::world().get_room(Location(Vnum(ROOM_VNUM_TICKETBACKROOM)));

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
		    ch->pcdata->questpoints, ch->pcdata->skillpoints);
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

			if (ch->is_npc() && ch->act_flags.has(ACT_PET)) {
				check_social(questman, "rofl", ch->name);

				buf = Format::format("Who ever heard of a pet questing for its %s?",
				        GET_ATTR_SEX(ch) == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != nullptr)
					check_social(questman, "laugh", ch->leader->name);

				return;
			}

			if (IS_SQUESTOR(ch)) {
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
			buf = Format::format("You have %d minutes to complete this quest.", ch->pcdata->sqcountdown);
			do_say(questman, buf);
			do_say(questman, "May the gods go with you!");
			wiznet("{Y:SKILL QUEST:{x $N has begun a skill quest", ch, nullptr, WIZ_QUEST, 0, 0);
			return;
		}

		if ((questman = find_questmaster(ch)) != nullptr) {
			act("$n asks $N for a quest.", ch, nullptr, questman, TO_ROOM);
			act("You ask $N for a quest.", ch, nullptr, questman, TO_CHAR);

			if (ch->is_npc() && ch->act_flags.has(ACT_PET)) {
				check_social(questman, "rofl", ch->name);

				buf = Format::format("Who ever heard of a pet questing for its %s?",
				        GET_ATTR_SEX(ch) == 2 ? "mistress" : "master");
				do_say(questman, buf);

				if (ch->leader != nullptr)
					check_social(questman, "laugh", ch->leader->name);

				return;
			}

			if (IS_QUESTOR(ch)) {
				do_say(questman, "But you're already on a quest!");
				return;
			}

			if (ch->pcdata->nextquest > 0) {
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_ROOM);
				act("$N says 'You're very brave, $n, but let someone else have a chance.'", ch, nullptr, questman, TO_CHAR);
				return;
			}

			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_ROOM);
			act("$N says 'Thank you, brave $n!", ch, nullptr, questman, TO_CHAR);
			ch->pcdata->questmob = 0;
			ch->pcdata->questobj = 0;
			ch->pcdata->questobf = 0;
			ch->pcdata->questloc = Location();
			generate_quest(ch, questman);

			if (ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0) {
				ch->pcdata->plr_flags += PLR_QUESTOR;
				buf = Format::format("You have %d minutes to complete this quest.", ch->pcdata->countdown);
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
		if (!IS_QUESTOR(ch)) {
			stc("You aren't currently on a quest.\n", ch);

			if (ch->pcdata->nextquest > 1)
				ptc(ch, "There are %d minutes remaining until you can quest again.\n",
				    ch->pcdata->nextquest);
			else if (ch->pcdata->nextquest == 1)
				stc("There is less than a minute remaining until you can quest again.\n", ch);
		}
		else if (ch->pcdata->countdown > 0)
			ptc(ch, "You have %d minutes left to complete your current quest.\n", ch->pcdata->countdown);

		if (!IS_SQUESTOR(ch)) {
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

#include "PointQuest.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Character.hh"
#include "MobilePrototype.hh"
#include "merc.hh"
#include "Area.hh"
#include "Affect.hh"
#include "interp.hh"
#include "channels.hh"


/* Object vnums for object quest 'tokens' */
#define QUEST_OBJQUEST1 1283
#define QUEST_OBJQUEST2 1284
#define QUEST_OBJQUEST3 1285
#define QUEST_OBJQUEST4 1286
#define QUEST_OBJQUEST5 1287

void PointQuest::
complete_notify(Character *ch, const QuestTargetable *t) const {
	stc("{YYou have almost completed your QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
	String buf = Format::format("{Y:QUEST: {x$N has found %s", String(t->identifier()));
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
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

PointQuest * PointQuest::
generate(Character *ch, Character *questman) {
	/*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */
	/*  Here is the new, improved, align-respecting version of the quest
	victim selection process. It requires at least twice as much CPU
	time as the previous version, so I hope the results are worth it! */

	/*  The following outer while(1) loop is needed to go through the mob
	selection twice: Once to count the total number of candidate mobs
	(fulfilling level and other criteria) and the number of suitably
	aligned candidate mobs; and the second (indicated by either pass 2
	or pass 3) to find a randomly chosen n-th aligned candidate or
	candidate. */
	int pass = 1;
	int target = 0;
	Character *victim;

	while (pass != 0) {
		int aligned = 0;
		int total = 0;

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
		ch->pcdata->quests.nextquest = 1;
		return nullptr;
	}

	/* at this point the player is sure to get a quest */
	RoomPrototype *room = victim->in_room;
	int countdown = number_range(10, 30);
	String buf;

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

		Object *questitem = create_object(get_obj_index(objvnum), ch->level);

		if (! questitem) {
			bug("Error creating quest item.", 0);
			return nullptr;
		}

		/* Add player's name to mox name to prevent visibility by others */
		Format::sprintf(buf, "%s %s", questitem->name, ch->name);
		questitem->name = buf;

		/* Mox timer added by Demonfire as a preventative measure against cheating.
		The countdown timer assignment was moved here so that it could be used
		in the mox timer calculation, it was normally assigned after the return
		of this function. */
		questitem->timer = (4 * ch->pcdata->quests.quest->countdown + 10) / 3;

		obj_to_room(questitem, room);

		Format::sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr);
		do_say(questman, buf);
		do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
		Format::sprintf(buf, "Look in the general area of %s for %s!", room->area->name, room->name);
		do_say(questman, buf);

		// build the quest
		QuestTarget target(
			QuestTarget::Obj,
			room->vnum,
			questitem
		);

		PointQuest *quest = new PointQuest(
			questman->pIndexData->vnum,
			countdown
		);

		quest->targets.push_back(target);
		return quest;
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

		// build the quest
		QuestTarget target(
			QuestTarget::MobVnum,
			room->vnum,
			victim->pIndexData
		);

		PointQuest *quest = new PointQuest(
			questman->pIndexData->vnum,
			countdown
		);

		quest->targets.push_back(target);
		return quest;
	}
} /* end generate_quest() */

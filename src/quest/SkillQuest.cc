#include "quest/SkillQuest.hh"
#include "merc.hh"
#include "channels.hh"
#include "interp.hh"

namespace quest {

bool SkillQuest::
complete_objective(Character *ch, const QuestTargetable *t) const {
	QuestTarget& target == next_target();


	if (!is_next_target(t))
		return;

	if (is_complete()) { // found both things
		stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
		stc("{YReturn to the questmistress before your time runs out!{x\n", ch);
	}
	else {
		const Character *mob;
		for (const QuestTarget& target: targets)
			if (target.type == QuestTarget::Mob)
				mob = dynamic_cast<const Character *>(target.target);

		if (mob == nullptr) {
			bugf("complete_notify: half complete with no valid mob");
			return;
		}

		stc("{YYou have completed part of your {VSKILL QUEST!{x\n", ch);
		ptc(ch, "{YTake the artifact to %s while there is still time!{x\n",
		    mob->short_descr);
	}

	String buf;
	Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has found the %s", String(t->identifier()));
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
}

bool SkillQuest::
mob_found() const {
	for (const QuestTarget& target: targets)
		if (target.type == QuestTarget::Mob && target.is_complete)
			return true;

	return false;
}

bool SkillQuest::
obj_found() const {
	for (const QuestTarget& target: targets)
		if (target.type == QuestTarget::Obj && target.is_complete)
			return true;

	return false;
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

void SkillQuest::
squestobj_to_squestmob(Character *ch, Object *obj, Character *mob) {
	String buf;
	check_social(mob, "beam", ch->name.c_str());
	Format::sprintf(buf, "Thank you, %s, for returning my lost %s!", ch->name, obj->short_descr);
	do_say(mob, buf);
	do_say(mob, "I left the gold reward with the Questmistress.  Farewell, and thank you!");
	act("$n turns and walks away.", mob, nullptr, nullptr, TO_ROOM);
	stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
	stc("{YReturn to the questmaster before your time runs out!{x\n", ch);

	Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has returned %s to %s", obj->short_descr, mob->short_descr);
	wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
}

void notify(event::Type type, event::EventArgs& args) {
	Character *ch = args["ch"];

	if (IS_NPC(ch))
		return;

	Quest *quest = ch->pcdata->quests.squest;

	if (!quest)
		return;

	QuestTargetable *target = nullptr;

	switch (type) {
		case event::character_give_obj: {
			Object *obj = args["obj"];
			Character *mob = args["victim"];

			if (!quest->is_target(obj)
			 || !quest->is_target(mob))
				return;

			squestobj_to_squestmob(ch, obj, mob);

			if (!complete_objective(ch, mob))
				return;

			event::fire(skill_quest_obj_to_mob, {
				{"ch", ch}, {"obj", obj}, {"mob", mob}
			});
			extract_obj(obj);
			extract_char(mob, TRUE);
			break;
		}

		case event::object_to_char: {
			Object *obj = args["obj"];

			if (!complete_objective(ch, obj))
				return;

			event::fire(skill_quest_found_obj, {
				{"ch", ch}, {"obj", obj}
			});
			extract_obj(obj);
			break;
		}
		case event::character_to_room: {
			Character *mob = args["victim"];

			if (!complete_objective(ch, mob))
				return;

			event::fire(skill_quest_found_mob, {
				{"ch", ch}, {"mob", mob}
			});
			extract_char(mob, TRUE);
			break;
		}
	}
}

} // namespace quest

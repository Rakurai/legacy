#include "SkillQuest.hh"
#include "merc.hh"
#include "channels.hh"
#include "interp.hh"

void SkillQuest::
complete_notify(Character *ch, const QuestTargetable *t) const {
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

void SkillQuest::
squestobj_to_squestmob(Character *ch, Object *obj, Character *mob) {
	if (IS_NPC(ch))
		return;

	Quest *quest = ch->pcdata->quests.squest;

	if (!quest
	 || !quest->is_target(obj)
	 || !quest->is_target(mob))
		return;

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

	extract_obj(obj);
	extract_char(mob, TRUE);

	quest->check_complete(ch, mob);
}

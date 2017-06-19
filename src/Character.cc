#include "Character.hh"

#include "act.hh"
#include "affect_list.hh"
#include "Affect.hh"
#include "Area.hh"
#include "Edit.hh"
#include "Exit.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "random.hh"
#include "recycle.hh"
#include "RoomPrototype.hh"
#include "Tail.hh"

Character::Character() {
	logon = current_time;
	lines = PAGELEN;

	for (int i = 0; i < 4; i++)
		armor_base[i]            = 100;

	position = POS_STANDING;
	hit  = ATTR_BASE(this, APPLY_HIT)                 = 20;
	mana = ATTR_BASE(this, APPLY_MANA)                = 100;
	mana = ATTR_BASE(this, APPLY_STAM)                = 100;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(this, stat_to_attr(stat)) = 13;
}

Character::~Character() {
	if (IS_NPC(this))
		mobile_count--;

	Object *obj, *obj_next;
	for (obj = carrying; obj != nullptr; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	if (pcdata) {
		for (obj = pcdata->locker; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		for (obj = pcdata->strongbox; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}
	}

	affect_clear_list(&affected);

	/* stop active TAILs, if any -- Elrac */
	if (!IS_NPC(this) && pcdata && pcdata->tailing)
		set_tail(this, nullptr, TAIL_NONE);

	/* stop all passive TAILs -- Elrac */
	for (Tail *td = tail; td != nullptr; td = tail) {
		tail = td->next;
		act("You stop tailing $n", this, nullptr, td->tailed_by, TO_VICT, POS_SLEEPING, FALSE);
		delete td;
	}

	tail = nullptr;

	if (edit != nullptr) {
		delete edit;
		edit = nullptr;
	}

	if (pcdata != nullptr)
		free_pcdata(pcdata);
	if (gen_data != nullptr)
		free_gen_data(gen_data);
	if (apply_cache)
		delete[] apply_cache;
	if (defense_mod)
		delete[] defense_mod;
	if (affect_cache)
		free_affect_cache(this);
}

void Character::update() {
	Character *ch = this;

	if (!IS_NPC(ch) || ch->in_room == nullptr || affect_exists_on_char(ch, gsn_charm_person))
		return;

	if (get_position(ch) <= POS_SITTING)
		return;

	/* Why check for resting mobiles? */

	if (ch->in_room->area->empty)
		/* && !ch->act_flags.has(ACT_UPDATE_ALWAYS)) */
		return;

	/* Examine call for special procedure */
	if (ch->spec_fun != 0) {
		if ((*ch->spec_fun)(ch))
			return;
	}

	if (ch->pIndexData->pShop != nullptr)
		if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth) {
			ch->gold += ch->pIndexData->wealth * number_range(1, 20) / 5000000;
			ch->silver += ch->pIndexData->wealth * number_range(1, 20) / 50000;
		}

	/* MOBprogram random trigger */
	if (ch->in_room->area->nplayer > 0) {
		mprog_random_trigger(ch);

		/* If ch dies or changes
		position due to it's random
		trigger, continue - Kahn */
		if (get_position(ch) < POS_STANDING)
			return;
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if (get_position(ch) != POS_STANDING)
		return;

	/* Scavenge */
	if (ch->act_flags.has(ACT_SCAVENGER)
	    && ch->in_room->contents != nullptr
	    && number_bits(6) == 0) {
		Character *gch;
		Object *obj;
		Object *obj_best = 0;
		bool not_used;
		int max = 1;

		for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
			not_used = TRUE;

			for (gch = obj->in_room->people; gch != nullptr; gch = gch->next_in_room)
				if (gch->on == obj)
					not_used = FALSE;

			if (CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj) && obj->cost > max && not_used) {
				obj_best = obj;
				max = obj->cost;
			}
		}

		if (obj_best) {
			obj_from_room(obj_best);
			obj_to_char(obj_best, ch);
			act("$n gets $p.", ch, obj_best, nullptr, TO_ROOM);
		}
	}

	int door;
	Exit *pexit;

	/* Wander */
	if (!ch->act_flags.has(ACT_SENTINEL)
	    && number_bits(3) == 0
	    && (door = number_bits(5)) <= 5
	    && (pexit = ch->in_room->exit[door]) != nullptr
	    &&   pexit->u1.to_room != nullptr
	    &&   !pexit->exit_flags.has(EX_CLOSED)
	    &&   !GET_ROOM_FLAGS(pexit->u1.to_room).has(ROOM_NO_MOB)
	    && (!ch->act_flags.has(ACT_STAY_AREA)
	        ||   pexit->u1.to_room->area == ch->in_room->area)
	    && (!ch->act_flags.has(ACT_OUTDOORS)
	        ||   !GET_ROOM_FLAGS(pexit->u1.to_room).has(ROOM_INDOORS))
	    && (!ch->act_flags.has(ACT_INDOORS)
	        ||   GET_ROOM_FLAGS(pexit->u1.to_room).has(ROOM_INDOORS))) {
		move_char(ch, door, FALSE);

		/* If ch changes position due
		to it's or someother mob's
		movement via MOBProgs,
		continue - Kahn */
		if (get_position(ch) < POS_STANDING)
			return;
	}

}

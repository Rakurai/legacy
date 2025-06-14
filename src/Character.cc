#include "Character.hh"

#include "act.hh"
#include "affect/affect_list.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Edit.hh"
#include "Exit.hh"
#include "Game.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "random.hh"
#include "Room.hh"
#include "Tail.hh"
#include "MobProg.hh"

Character::Character() {
	logon = Game::current_time;
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

	affect::clear_list(&affected);

	/* stop active TAILs, if any -- Elrac */
	if (!this->is_npc() && pcdata && pcdata->tailing)
		set_tail(this, nullptr, TAIL_NONE);

	/* stop all passive TAILs -- Elrac */
	for (Tail *td = tail; td != nullptr; td = tail) {
		tail = td->next;
		act("You stop tailing $n", this, nullptr, td->tailed_by, TO_VICT, POS_SLEEPING, false);
		delete td;
	}

	tail = nullptr;

	if (edit != nullptr) {
		delete edit;
		edit = nullptr;
	}

	if (pcdata != nullptr)
		delete pcdata;
	if (apply_cache)
		delete[] apply_cache;
	if (defense_mod)
		delete[] defense_mod;
	if (affect_cache)
		affect::free_cache(this);
}

void Character::update() {
	Character *ch = this;

	if (!ch->is_npc() || ch->in_room == nullptr || affect::exists_on_char(ch, affect::type::charm_person))
		return;

	if (get_position(ch) <= POS_SITTING)
		return;

	/* Why check for resting mobiles? */

	if (ch->in_room->area().num_players() == 0
	 && ch->in_room->area().num_imms() == 0)
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
//	if (!ch->in_room->area().is_empty()) {
		mprog_random_trigger(ch);

		/* If ch dies or changes
		position due to it's random
		trigger, continue - Kahn */
		if (ch->is_garbage())
			return;

		mprog_random_area_trigger(ch);

		if (ch->is_garbage())
			return;
//	}

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
			not_used = true;

			for (gch = obj->in_room->people; gch != nullptr; gch = gch->next_in_room)
				if (gch->on == obj)
					not_used = false;

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
	    &&   pexit->to_room != nullptr
	    &&   !pexit->exit_flags.has(EX_CLOSED)
	    &&   !pexit->to_room->flags().has(ROOM_NO_MOB)
	    && (!ch->act_flags.has(ACT_STAY_AREA)
	        ||   pexit->to_room->area() == ch->in_room->area())
	    && (!ch->act_flags.has(ACT_OUTDOORS)
	        ||   !pexit->to_room->flags().has(ROOM_INDOORS))
	    && (!ch->act_flags.has(ACT_INDOORS)
	        ||   pexit->to_room->flags().has(ROOM_INDOORS))) {
		move_char(ch, door, false);

		/* If ch changes position due
		to it's or someother mob's
		movement via MOBProgs,
		continue - Kahn */
		if (get_position(ch) < POS_STANDING)
			return;
	}
}

bool Character::
has_cgroup(const Flags& cg) const {
	return this->is_npc() ? false : pcdata->cgroup_flags.has_any_of(cg);
}

void Character::
add_cgroup(const Flags& cg) {
	if (!this->is_npc())
		pcdata->cgroup_flags += cg;
}

void Character::
remove_cgroup(const Flags& cg) {
	if (!this->is_npc())
		pcdata->cgroup_flags -= cg;
}

//changed macros for MAX_HIT, MAX_STAM, MAX_MANA , GET_ATTR_AC and GET_AC into functions
int GET_MAX_HIT(Character *ch)
{
	int result = 0;
	int base = (GET_ATTR(ch, APPLY_HIT));
	
	if (GET_ATTR(ch, APPLY_HIT) < 1){
		result = 1;
		return result;
	}
	if (GET_ATTR(ch, APPLY_HIT) > 30000){
		result = 30000;
		return result;
	}
	if (GET_ATTR(ch, SET_PALADIN_GRACE) >= 4){
		result = (base + (base * 20 / 100));
		if (result > 30000)
			result = 30000;
		return result;
	}
	
	return base;
}

int GET_MAX_MANA(Character *ch)
{
	int result = 0;
	int base = (GET_ATTR(ch, APPLY_MANA));
	
	if (GET_ATTR(ch, APPLY_MANA) < 1){
		result = 1;
		return result;
	}
	if (GET_ATTR(ch, APPLY_MANA) > 30000){
		result = 30000;
		return result;
	}

	return base;
}

int GET_MAX_STAM(Character *ch)
{
	int result = 0;
	int base = (GET_ATTR(ch, APPLY_STAM));
	
	if (GET_ATTR(ch, APPLY_STAM) < 1){
		result = 1;
		return result;
	}
	if (GET_ATTR(ch, APPLY_STAM) > 30000){
		result = 30000;
		return result;
	}

	return base;
}

int GET_ATTR_AC(Character *ch)
{
	int base 	= GET_ATTR(ch, APPLY_AC);
	int mod 	= 0;
	
	if (IS_AWAKE(ch))
		base += dex_app[GET_ATTR_DEX(ch)].defensive;
	if (!ch->is_npc() && ch->pcdata->remort_count > 0)
		base -= (ch->pcdata->remort_count * ch->level / 50); //-1 per 10 levels, -1 per 5 remorts - Montrey
	if (!ch->is_npc() && GET_ATTR(ch, SET_PALADIN_GRACE) >= 3)
		base += (base * 20 / 100 );
	
	return base;
}

int GET_AC(Character *ch, int type)
{
	return ch->armor_base[type] + GET_ATTR_AC(ch);
}
	
	
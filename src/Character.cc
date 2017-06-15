#include "Character.hh"
#include "merc.hh"
#include "affect_list.hh"
#include "recycle.hh"
#include "act.hh"

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

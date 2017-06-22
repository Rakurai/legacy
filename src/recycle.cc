#include "Affect.hh"
#include "Character.hh"
#include "Customize.hh"
#include "Descriptor.hh"
#include "Duel.hh"
#include "ExtraDescr.hh"
#include "merc.hh"
#include "Mercenary.hh"
#include "Object.hh"
#include "Player.hh"
#include "War.hh"

/* stuff for setting ids */
long    last_pc_id;
long    last_mob_id;

long get_pc_id(void)
{
	int val;
	val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
	last_pc_id = val;
	return val;
}

long get_mob_id(void)
{
	last_mob_id++;
	return last_mob_id;
}

/* descriptor recycling */
Descriptor	*new_descriptor() { return new Descriptor(); }
void free_descriptor(Descriptor *obj) { delete obj; }

Customize *new_gen_data() { return new Customize(); }
void free_gen_data(Customize *obj) { delete obj; }

Affect	*new_affect() { return new Affect(); }
void free_affect(Affect *obj) { delete obj; }

Object *new_obj() { return new Object(); }
void free_obj(Object *obj) { delete obj; }

Player	*new_pcdata() { return new Player(); }
void free_pcdata(Player *obj) { delete obj; }

War *new_war() { return new War(); ; }
void free_war(War *obj) { delete obj; }

War::Opponent *new_opp() { return new War::Opponent(); }
void free_opp(War::Opponent *obj) { delete obj; }

War::Event *new_event() { return new War::Event(); }
void free_event(War::Event *obj) { delete obj; }

Mercenary *new_merc() { return new Mercenary(); }
void free_merc(Mercenary *obj) { delete obj; }

Mercenary::Offer *new_offer() { return new Mercenary::Offer(); }
void free_offer(Mercenary::Offer *obj) { delete obj; }

Duel *new_duel() { return new Duel(); }
void free_duel(Duel *obj) { delete obj; }

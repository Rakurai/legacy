/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "merc.h"
#include "recycle.h"
#include "affect.h"
#include "affect_list.h"
#include "memory.h"

/* CHANGES
   We keep having problems with assumptions about whether data needs to be zeroed or set
   to some default value before it is used, resulting in a lot of crashes or accessing
   garbage data.  To be clear: new_* is contractually obligated to return a piece of memory
   that is ZEROED with the exception of default values, and all Strings will be empty.
     -- Montrey 2016 (removed str_dup/free_string 2017)
 */

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
	DESCRIPTOR_DATA *d;

	if (descriptor_free == NULL)
		d = new DESCRIPTOR_DATA;
	else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
	}

	*d = (DESCRIPTOR_DATA){0};
	d->connected     = CON_GET_NAME;
	d->outbuf.erase();
	d->host.erase();

	VALIDATE(d);
	return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
	if (!IS_VALID(d))
		return;

	INVALIDATE(d);
	d->next = descriptor_free;
	descriptor_free = d;
}

/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;

GEN_DATA *new_gen_data(void)
{
	GEN_DATA *gen;

	if (gen_data_free == NULL)
		gen = new GEN_DATA;
	else {
		gen = gen_data_free;
		gen_data_free = gen_data_free->next;
	}

	*gen = (GEN_DATA){0};

	VALIDATE(gen);
	return gen;
}

void free_gen_data(GEN_DATA *gen)
{
	if (!IS_VALID(gen))
		return;

	INVALIDATE(gen);
	gen->next = gen_data_free;
	gen_data_free = gen;
}

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void)
{
	EXTRA_DESCR_DATA *ed;

	if (extra_descr_free == NULL)
		ed = new EXTRA_DESCR_DATA;
	else {
		ed = extra_descr_free;
		extra_descr_free = extra_descr_free->next;
	}

	*ed = (EXTRA_DESCR_DATA){0};
	ed->keyword.erase();
	ed->description.erase();

	VALIDATE(ed);
	return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
	if (!IS_VALID(ed))
		return;

	INVALIDATE(ed);
	ed->next = extra_descr_free;
	extra_descr_free = ed;
}

/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
	AFFECT_DATA *af;

	if (affect_free == NULL)
		af = new AFFECT_DATA();
	else {
		af = affect_free;
		affect_free = affect_free->next;
	}

	*af = (AFFECT_DATA){0};
	VALIDATE(af);
	return af;
}

void free_affect(AFFECT_DATA *af)
{
	if (!IS_VALID(af))
		return;

	INVALIDATE(af);
	af->next = affect_free;
	affect_free = af;
}


/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
	OBJ_DATA *obj;

	if (obj_free == NULL)
		obj = new OBJ_DATA;
	else {
		obj = obj_free;
		obj_free = obj_free->next;
	}

	obj->name.erase();
	obj->description.erase();
	obj->short_descr.erase();
	obj->owner.erase();
	obj->material.erase();

	VALIDATE(obj);
	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	EXTRA_DESCR_DATA *ed, *ed_next;

	if (!IS_VALID(obj))
		return;

	// data wholly owned by this obj
	affect_clear_list(&obj->affected);
	affect_clear_list(&obj->gem_affected);

	for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
		ed_next = ed->next;
		free_extra_descr(ed);
	}

	INVALIDATE(obj);
	obj->next   = obj_free;
	obj_free    = obj;
}

/* stuff for recyling characters */
CHAR_DATA *char_free;

CHAR_DATA *new_char(void)
{
	CHAR_DATA *ch;

	if (char_free == NULL)
		ch = new CHAR_DATA;
	else {
		ch = char_free;
		char_free = char_free->next;
	}

	ch->name =
	ch->short_descr =
	ch->long_descr =
	ch->description =
	ch->prompt =
	ch->material =
	ch->prefix                 .erase();
	ch->logon                   = current_time;
	ch->lines                   = PAGELEN;

	for (int i = 0; i < 4; i++)
		ch->armor_base[i]            = 100;

	ch->position                = POS_STANDING;
	ch->hit  = ATTR_BASE(ch, APPLY_HIT)                 = 20;
	ch->mana = ATTR_BASE(ch, APPLY_MANA)                = 100;
	ch->mana = ATTR_BASE(ch, APPLY_STAM)                = 100;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(ch, stat_to_attr(stat)) = 13;

	VALIDATE(ch);
	return ch;
}

void free_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj, *obj_next;
	TAIL_DATA *td;

	if (!IS_VALID(ch))
		return;

	if (IS_NPC(ch))
		mobile_count--;

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	if (ch->pcdata) {
		for (obj = ch->pcdata->locker; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		for (obj = ch->pcdata->strongbox; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}
	}

	affect_clear_list(&ch->affected);

	/* stop active TAILs, if any -- Elrac */
	if (!IS_NPC(ch) && ch->pcdata && ch->pcdata->tailing)
		set_tail(ch, NULL, 0);

	/* stop all passive TAILs -- Elrac */
	for (td = ch->tail; td != NULL; td = ch->tail) {
		ch->tail = td->next;
		act("You stop tailing $n", ch, NULL, td->tailed_by, TO_VICT, POS_SLEEPING, FALSE);
		delete td;
	}

	ch->tail = NULL;

	if (ch->edit != NULL) {
		delete ch->edit;
		ch->edit = NULL;
	}

	if (ch->pcdata != NULL)
		free_pcdata(ch->pcdata);
	if (ch->gen_data != NULL)
		free_gen_data(ch->gen_data);
	if (ch->apply_cache)
		delete[] ch->apply_cache;
	if (ch->defense_mod)
		delete[] ch->defense_mod;
	if (ch->affect_cache)
		free_affect_cache(ch);

	INVALIDATE(ch);
	ch->next = char_free;
	char_free  = ch;
	return;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
	PC_DATA *pcdata;

	if (pcdata_free == NULL)
		pcdata = new PC_DATA;
	else {
		pcdata = pcdata_free;
		pcdata_free = pcdata_free->next;
	}

	*pcdata = (PC_DATA){0};

	// NO NULL STRINGS ALLOWED!!! -- Montrey
	pcdata->pwd =
	pcdata->bamfin =
	pcdata->bamfout =
	pcdata->gamein =
	pcdata->gameout =
	pcdata->title =
	pcdata->rank =
	pcdata->deity =
	pcdata->status =
	pcdata->last_lsite =
	pcdata->afk =
	pcdata->immname =
	pcdata->immprefix =
	pcdata->aura =
	pcdata->spouse =
	pcdata->propose =
	pcdata->whisper =
	pcdata->fingerinfo =
	pcdata->email.erase();


	VALIDATE(pcdata);
	return pcdata;
} /* end new_pcdata() */

void free_pcdata(PC_DATA *pcdata)
{
	if (!IS_VALID(pcdata))
		return;


	INVALIDATE(pcdata);
	pcdata->next = pcdata_free;
	pcdata_free = pcdata;
	return;
}

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

/* recycle war structures */
WAR_DATA *war_free;

WAR_DATA *new_war(void)
{
	WAR_DATA *war;

	if (war_free == NULL)
		war = new WAR_DATA;
	else {
		war = war_free;
		war_free = war_free->next;
	}

	*war = (WAR_DATA){{0}}; // weird double braces needed because of bugged gcc warning

	for (int i = 0; i < 4; i++) {
		war->chal[i] = new_opp();
		war->def[i] = new_opp();
	}

	VALIDATE(war);
	return war;
}

void free_war(WAR_DATA *war)
{
	EVENT_DATA *event, *event_next;

	if (!IS_VALID(war))
		return;

	for (int i = 0; i < 4; i++) {
		free_opp(war->chal[i]);
		free_opp(war->def[i]);
	}

	if (war->events != NULL)
		for (event = war->events; event != NULL; event = event_next) {
			event_next = event->next;
			free_event(event);
		}

	INVALIDATE(war);
	war->next = war_free;
	war->previous = war_free;
	war_free = war;
}

/* recycle war opponent structures */
OPP_DATA *opp_free;

OPP_DATA *new_opp(void)
{
	OPP_DATA *opp;

	if (opp_free == NULL)
		opp = new OPP_DATA;
	else {
		opp = opp_free;
		opp_free = opp_free->next;
	}

	*opp = (OPP_DATA){0};
	opp->name.erase();
	opp->clanname.erase();

	VALIDATE(opp);
	return opp;
}

void free_opp(OPP_DATA *opp)
{
	if (!IS_VALID(opp))
		return;

	INVALIDATE(opp);
	opp->next = opp_free;
	opp_free = opp;
}

/* recycle war event structures */
EVENT_DATA *event_free;

EVENT_DATA *new_event(void)
{
	EVENT_DATA *event;

	if (event_free == NULL)
		event = new EVENT_DATA;
	else {
		event = event_free;
		event_free = event_free->next;
	}

	*event = (EVENT_DATA){0};
	event->astr.erase();
	event->bstr.erase();

	VALIDATE(event);
	return event;
}

void free_event(EVENT_DATA *event)
{
	if (!IS_VALID(event))
		return;

	INVALIDATE(event);
	event->next = event_free;
	event_free = event;
}

/* recycle merc structures */
MERC_DATA *merc_free;

MERC_DATA *new_merc(void)
{
	MERC_DATA *merc;

	if (merc_free == NULL)
		merc = new MERC_DATA;
	else {
		merc = merc_free;
		merc_free = merc_free->next;
	}

	*merc = (MERC_DATA){0};
	merc->name.erase();
	merc->employer.erase();

	VALIDATE(merc);
	return merc;
}

void free_merc(MERC_DATA *merc)
{
	OFFER_DATA *offer, *offer_next;

	if (!IS_VALID(merc))
		return;

	for (offer = merc->offer; offer != NULL; offer = offer_next) {
		offer_next = offer->next;
		free_offer(offer);
	}

	INVALIDATE(merc);
	merc->next = merc_free;
	merc->previous = merc_free;
	merc_free = merc;
}

/* recycle merc offer structures */
OFFER_DATA *offer_free;

OFFER_DATA *new_offer(void)
{
	OFFER_DATA *offer;

	if (offer_free == NULL)
		offer = new OFFER_DATA;
	else {
		offer = offer_free;
		offer_free = offer_free->next;
	}

	*offer = (OFFER_DATA){0};
	offer->name.erase();

	VALIDATE(offer);
	return offer;
}

void free_offer(OFFER_DATA *offer)
{
	if (!IS_VALID(offer))
		return;

	INVALIDATE(offer);
	offer->next = offer_free;
	offer_free = offer;
}

/* recycle duel structures */
DUEL_DATA *duel_free;

DUEL_DATA *new_duel(void)
{
	DUEL_DATA *duel;

	if (duel_free == NULL)
		duel = new DUEL_DATA;
	else {
		duel = duel_free;
		duel_free = duel_free->next;
	}

	*duel = (DUEL_DATA){0};
	VALIDATE(duel);
	return duel;
}

void free_duel(DUEL_DATA *duel)
{
	if (!IS_VALID(duel))
		return;

	INVALIDATE(duel);
	duel->next = duel_free;
	duel->previous = duel_free;
	duel_free = duel;
}

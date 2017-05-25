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
#include "buffer.h"
#include "memory.h"

/* CHANGES
   We keep having problems with assumptions about whether data needs to be zeroed or set
   to some default value before it is used, resulting in a lot of crashes or accessing
   garbage data.  To be clear: new_* is contractually obligated to return a piece of memory
   that is ZEROED with the exception of default values, and all char* pointers will point to
   str_empty.  free_* on the other hand, will free ALL char* pointers in the object (freeing
   str_empty is fine).  Therefore, I have also gone through and removed a lot of useless
   str_dup("") after calls to new_*, and hopefully this will fix a few bugs where someone
   forgot to zero an array before using it.  -- Montrey 2016
*/

/* semiperm string recycling */
SEMIPERM *semiperm_free;

SEMIPERM *new_semiperm()
{
	SEMIPERM *semiperm;

	if (semiperm_free == NULL)
		semiperm = (SEMIPERM *)alloc_perm2(sizeof(*semiperm), "Semiperm");
	else {
		semiperm = semiperm_free;
		semiperm_free = semiperm_free->next;
	}

	*semiperm = (SEMIPERM){0};
	semiperm->string = str_empty;

	VALIDATE(semiperm);
	return semiperm;
}

void free_semiperm(SEMIPERM *semiperm)
{
	if (!IS_VALID(semiperm))
		return;

	free_string(semiperm->string);

	INVALIDATE(semiperm);
	semiperm->next = semiperm_free;
	semiperm_free = semiperm;
}

/* stuff for recyling notes */
NOTE_DATA *note_free;

NOTE_DATA *new_note()
{
	NOTE_DATA *note;

	if (note_free == NULL)
		note = (NOTE_DATA *)alloc_perm2(sizeof(*note), "Note");
	else {
		note = note_free;
		note_free = note_free->next;
	}

	*note = (NOTE_DATA){0};
	note->sender = str_empty;
	note->to_list = str_empty;
	note->subject = str_empty;
	note->date = str_empty;
	note->text = str_empty;

	VALIDATE(note);
	return note;
}

void free_note(NOTE_DATA *note)
{
	if (!IS_VALID(note))
		return;

	free_string(note->sender);
	free_string(note->to_list);
	free_string(note->subject);
	free_string(note->date);
	free_string(note->text);

	INVALIDATE(note);
	note->next = note_free;
	note_free   = note;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
	DESCRIPTOR_DATA *d;

	if (descriptor_free == NULL)
		d = (DESCRIPTOR_DATA *)alloc_perm2(sizeof(*d), "Descriptor");
	else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
	}

	*d = (DESCRIPTOR_DATA){0};
	d->connected     = CON_GET_NAME;
	d->outsize       = 2000;
	d->outbuf        = (char *)alloc_mem(d->outsize);
	d->host = str_empty;

	VALIDATE(d);
	return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
	if (!IS_VALID(d))
		return;

	free_string(d->host);
	free_mem(d->outbuf, d->outsize);

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
		gen = (GEN_DATA *)alloc_perm2(sizeof(*gen), "General");
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
		ed = (EXTRA_DESCR_DATA *)alloc_perm2(sizeof(*ed), "Extra Description");
	else {
		ed = extra_descr_free;
		extra_descr_free = extra_descr_free->next;
	}

	*ed = (EXTRA_DESCR_DATA){0};
	ed->keyword = str_empty;
	ed->description = str_empty;

	VALIDATE(ed);
	return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
	if (!IS_VALID(ed))
		return;

	free_string(ed->keyword);
	free_string(ed->description);

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
		af = (AFFECT_DATA *)alloc_perm2(sizeof(*af), "Affect");
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
		obj = (OBJ_DATA *)alloc_perm2(sizeof(*obj), "Object");
	else {
		obj = obj_free;
		obj_free = obj_free->next;
	}

	*obj = (OBJ_DATA){0};
	obj->name = str_empty;
	obj->description = str_empty;
	obj->short_descr = str_empty;
	obj->owner = str_empty;
	obj->material = str_empty;

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

	free_string(obj->name);
	free_string(obj->description);
	free_string(obj->short_descr);
	free_string(obj->owner);
	free_string(obj->material);

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
		ch = (CHAR_DATA *)alloc_perm2(sizeof(*ch), "Character");
	else {
		ch = char_free;
		char_free = char_free->next;
	}

	*ch                         = (CHAR_DATA){0};
	ch->name =
	ch->short_descr =
	ch->long_descr =
	ch->description =
	ch->prompt =
	ch->material =
	ch->prefix                  = str_empty;
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
		act_new("You stop tailing $n", ch, NULL, td->tailed_by, TO_VICT, POS_SLEEPING, FALSE);
		free_mem(td, sizeof(struct tail_data));
	}

	ch->tail = NULL;

	if (ch->edit != NULL) {
		free_mem(ch->edit, sizeof(EDIT_DATA));
		ch->edit = NULL;
	}

	if (ch->pcdata != NULL)
		free_pcdata(ch->pcdata);

	if (ch->apply_cache)
		free_mem(ch->apply_cache, APPLY_CACHE_MEM_SIZE);
	if (ch->defense_mod)
		free_mem(ch->defense_mod, DEFENSE_MOD_MEM_SIZE);
	if (ch->affect_cache)
		free_affect_cache(ch);

	free_string(ch->short_descr);
	free_string(ch->long_descr);
	free_string(ch->description);
	free_string(ch->prompt);
	free_string(ch->prefix);
	free_string(ch->material);
	// currently all 7 strings in char_data, make sure they're set to str_empty above

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
		pcdata = (PC_DATA *)alloc_perm2(sizeof(*pcdata), "PC Data");
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
	pcdata->email = str_empty;

	pcdata->buffer = new_buf();

	VALIDATE(pcdata);
	return pcdata;
} /* end new_pcdata() */

void free_pcdata(PC_DATA *pcdata)
{
	if (!IS_VALID(pcdata))
		return;

	free_string(pcdata->pwd);
	free_string(pcdata->bamfin);
	free_string(pcdata->bamfout);
	free_string(pcdata->gamein);
	free_string(pcdata->gameout);
	free_string(pcdata->title);
	free_string(pcdata->rank);
	free_string(pcdata->deity);
	free_string(pcdata->status);
	free_string(pcdata->last_lsite);
	free_string(pcdata->afk);
	free_string(pcdata->immname);
	free_string(pcdata->immprefix);
	free_string(pcdata->aura);
	free_string(pcdata->spouse);
	free_string(pcdata->propose);
	free_string(pcdata->whisper);
	free_string(pcdata->fingerinfo);
	free_string(pcdata->email);

	free_buf(pcdata->buffer);

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

MEM_DATA *mem_data_free;

MEM_DATA *new_mem_data(void)
{
	MEM_DATA *memory;

	if (mem_data_free == NULL)
		memory = (MEM_DATA *)alloc_mem(sizeof(*memory));
	else {
		memory = mem_data_free;
		mem_data_free = mem_data_free->next;
	}

	*memory = (MEM_DATA){0};

	VALIDATE(memory);
	return memory;
}

void free_mem_data(MEM_DATA *memory)
{
	if (!IS_VALID(memory))
		return;

	INVALIDATE(memory);
	memory->next = mem_data_free;
	mem_data_free = memory;
}

/* recycle war structures */
WAR_DATA *war_free;

WAR_DATA *new_war(void)
{
	WAR_DATA *war;

	if (war_free == NULL)
		war = (WAR_DATA *)alloc_perm2(sizeof(*war), "War");
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
		opp = (OPP_DATA *)alloc_perm2(sizeof(*opp), "War Opponent");
	else {
		opp = opp_free;
		opp_free = opp_free->next;
	}

	*opp = (OPP_DATA){0};
	opp->name = &str_empty[0];
	opp->clanname = &str_empty[0];

	VALIDATE(opp);
	return opp;
}

void free_opp(OPP_DATA *opp)
{
	if (!IS_VALID(opp))
		return;

	free_string(opp->name);
	free_string(opp->clanname);

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
		event = (EVENT_DATA *)alloc_perm2(sizeof(*event), "War Event");
	else {
		event = event_free;
		event_free = event_free->next;
	}

	*event = (EVENT_DATA){0};
	event->astr = str_empty;
	event->bstr = str_empty;

	VALIDATE(event);
	return event;
}

void free_event(EVENT_DATA *event)
{
	if (!IS_VALID(event))
		return;

	free_string(event->astr);
	free_string(event->bstr);

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
		merc = (MERC_DATA *)alloc_perm2(sizeof(*merc), "Merc");
	else {
		merc = merc_free;
		merc_free = merc_free->next;
	}

	*merc = (MERC_DATA){0};
	merc->name = str_empty;
	merc->employer = str_empty;

	VALIDATE(merc);
	return merc;
}

void free_merc(MERC_DATA *merc)
{
	OFFER_DATA *offer, *offer_next;

	if (!IS_VALID(merc))
		return;

	free_string(merc->name);
	free_string(merc->employer);

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
		offer = (OFFER_DATA *)alloc_perm2(sizeof(*offer), "Merc Offer");
	else {
		offer = offer_free;
		offer_free = offer_free->next;
	}

	*offer = (OFFER_DATA){0};
	offer->name = str_empty;

	VALIDATE(offer);
	return offer;
}

void free_offer(OFFER_DATA *offer)
{
	if (!IS_VALID(offer))
		return;

	free_string(offer->name);

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
		duel = (DUEL_DATA *)alloc_perm2(sizeof(*duel), "Duel");
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

/* recycle coordinate structures
WM_COORD_DATA *coord_free;

WM_COORD_DATA *new_coord(void)
{
        static WM_COORD_DATA coord_zero;
        WM_COORD_DATA *coord;

        if (coord_free == NULL)
                coord = alloc_perm2(sizeof(*coord),"Coordinate");
        else
        {
                coord = coord_free;
                coord_free = coord_free->next;
        }

        *coord = coord_zero;

        return coord;
}

void free_coord(WM_COORD_DATA *coord)
{
        coord->next = coord_free;
        coord->previous = coord_free;
        coord_free = coord;
} */

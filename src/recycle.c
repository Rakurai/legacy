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

extern CHAR_DATA *dv_char;
extern char dv_command[];
extern char *dv_where;

/* semiperm string recycling */
SEMIPERM *semiperm_free;

SEMIPERM *new_semiperm()
{
	SEMIPERM *semiperm;

	if (semiperm_free == NULL)
		semiperm = alloc_perm2(sizeof(*semiperm), "Semiperm");
	else {
		semiperm = semiperm_free;
		semiperm_free = semiperm_free->next;
	}

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
		note = alloc_perm2(sizeof(*note), "Note");
	else {
		note = note_free;
		note_free = note_free->next;
	}

	VALIDATE(note);
	return note;
}

void free_note(NOTE_DATA *note)
{
	if (!IS_VALID(note))
		return;

	free_string(note->text);
	free_string(note->subject);
	free_string(note->to_list);
	free_string(note->date);
	free_string(note->sender);
	INVALIDATE(note);
	note->next = note_free;
	note_free   = note;
}


/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
	static DESCRIPTOR_DATA d_zero;
	DESCRIPTOR_DATA *d;

	if (descriptor_free == NULL)
		d = alloc_perm2(sizeof(*d), "Descriptor");
	else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
	}

	*d = d_zero;
	VALIDATE(d);
	d->hostaddr      = 0L;
	d->connected     = CON_GET_NAME;
	d->showstr_head  = NULL;
	d->showstr_point = NULL;
	d->outsize       = 2000;
	d->outbuf        = alloc_mem(d->outsize);
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
	static GEN_DATA gen_zero;
	GEN_DATA *gen;

	if (gen_data_free == NULL)
		gen = alloc_perm2(sizeof(*gen), "General");
	else {
		gen = gen_data_free;
		gen_data_free = gen_data_free->next;
	}

	*gen = gen_zero;
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
		ed = alloc_perm2(sizeof(*ed), "Extra Description");
	else {
		ed = extra_descr_free;
		extra_descr_free = extra_descr_free->next;
	}

	ed->keyword = &str_empty[0];
	ed->description = &str_empty[0];
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
	static AFFECT_DATA af_zero;
	AFFECT_DATA *af;

	if (affect_free == NULL)
		af = alloc_perm2(sizeof(*af), "Affect");
	else {
		af = affect_free;
		affect_free = affect_free->next;
	}

	*af = af_zero;
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
	static OBJ_DATA obj_zero;
	OBJ_DATA *obj;

	if (obj_free == NULL)
		obj = alloc_perm2(sizeof(*obj), "Object");
	else {
		obj = obj_free;
		obj_free = obj_free->next;
	}

	*obj = obj_zero;
	VALIDATE(obj);
	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;
	EXTRA_DESCR_DATA *ed, *ed_next;

	if (!IS_VALID(obj))
		return;

	for (paf = obj->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		free_affect(paf);
	}

	obj->affected = NULL;

	for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
		ed_next = ed->next;
		free_extra_descr(ed);
	}

	obj->extra_descr = NULL;
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
	static CHAR_DATA ch_zero;
	CHAR_DATA *ch;
	int i;

	if (char_free == NULL)
		ch = alloc_perm2(sizeof(*ch), "Character");
	else {
		ch = char_free;
		char_free = char_free->next;
	}

	*ch                         = ch_zero;
	VALIDATE(ch);
	ch->name                    = &str_empty[0];
	ch->short_descr             = &str_empty[0];
	ch->long_descr              = &str_empty[0];
	ch->description             = &str_empty[0];
	ch->prompt                  = &str_empty[0];
	ch->prefix                  = &str_empty[0];
	ch->logon                   = current_time;
	ch->lines                   = PAGELEN;

	for (i = 0; i < 4; i++)
		ch->armor_a[i]            = 100;

	ch->position                = POS_STANDING;
	ch->hit                     = 20;
	ch->max_hit                 = 20;
	ch->mana                    = 100;
	ch->max_mana                = 100;
	ch->stam                    = 100;
	ch->max_stam                = 100;

	for (i = 0; i < MAX_STATS; i ++) {
		ch->perm_stat[i] = 13;
		ch->mod_stat[i] = 0;
	}

	return ch;
}


void free_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
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

	for (paf = ch->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(ch, paf);
	}

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

	free_string(ch->name);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	free_string(ch->description);
	free_string(ch->prompt);
	free_string(ch->prefix);
	free_string(ch->material);

	if (ch->pcdata != NULL)
		free_pcdata(ch->pcdata);

	ch->next = char_free;
	char_free  = ch;
	INVALIDATE(ch);
	return;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
	int alias;
	int query;
	static PC_DATA pcdata_zero;
	PC_DATA *pcdata;

	if (pcdata_free == NULL)
		pcdata = alloc_perm2(sizeof(*pcdata), "PC Data");
	else {
		pcdata = pcdata_free;
		pcdata_free = pcdata_free->next;
	}

	*pcdata = pcdata_zero;

	for (alias = 0; alias < MAX_ALIAS; alias++) {
		pcdata->alias[alias] = NULL;
		pcdata->alias_sub[alias] = NULL;
	}

	for (query = 0; query < MAX_QUERY; query++)
		pcdata->query[query] = NULL;

	pcdata->last_ltime = (time_t) 0;
	pcdata->last_saved = (time_t) 0;
	pcdata->buffer = new_buf();
	VALIDATE(pcdata);
	return pcdata;
} /* end new_pcdata() */


void free_pcdata(PC_DATA *pcdata)
{
	int alias;
	int query;

	if (!IS_VALID(pcdata))
		return;

	free_string(pcdata->pwd);
	free_string(pcdata->bamfin);
	free_string(pcdata->bamfout);
	free_string(pcdata->title);
	free_string(pcdata->rank);
	free_string(pcdata->deity);
	free_string(pcdata->status);

	/* free_string(pcdata->last_ltime); */
	/* free_string(pcdata->last_saved); */
	if (pcdata->last_lsite != NULL) free_string(pcdata->last_lsite); /* having probs with sandserver down on spawn */

	free_string(pcdata->afk);
	free_string(pcdata->immname);
	free_string(pcdata->spouse);
	free_string(pcdata->whisper);
	free_string(pcdata->fingerinfo);
	free_string(pcdata->email);
	free_buf(pcdata->buffer);

	for (alias = 0; alias < MAX_ALIAS; alias++) {
		free_string(pcdata->alias[alias]);
		free_string(pcdata->alias_sub[alias]);
	}

	for (query = 0; query < MAX_QUERY; query++)
		free_string(pcdata->query[query]);

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

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
	MEM_DATA *memory;

	if (mem_data_free == NULL)
		memory = alloc_mem(sizeof(*memory));
	else {
		memory = mem_data_free;
		mem_data_free = mem_data_free->next;
	}

	memory->next = NULL;
	memory->id = 0;
	memory->reaction = 0;
	memory->when = 0;
	VALIDATE(memory);
	return memory;
}

void free_mem_data(MEM_DATA *memory)
{
	if (!IS_VALID(memory))
		return;

	memory->next = mem_data_free;
	mem_data_free = memory;
	INVALIDATE(memory);
}

/* recycle war structures */
WAR_DATA *war_free;

WAR_DATA *new_war(void)
{
	static WAR_DATA war_zero;
	WAR_DATA *war;
	int i;

	if (war_free == NULL)
		war = alloc_perm2(sizeof(*war), "War");
	else {
		war = war_free;
		war_free = war_free->next;
	}

	*war = war_zero;

	for (i = 0; i < 4; i++) {
		war->chal[i] = new_opp();
		war->def[i] = new_opp();
	}

	VALIDATE(war);
	return war;
}

void free_war(WAR_DATA *war)
{
	EVENT_DATA *event, *event_next;
	int i;

	if (!IS_VALID(war))
		return;

	for (i = 0; i < 4; i++) {
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
	static OPP_DATA opp_zero;
	OPP_DATA *opp;

	if (opp_free == NULL)
		opp = alloc_perm2(sizeof(*opp), "War Opponent");
	else {
		opp = opp_free;
		opp_free = opp_free->next;
	}

	*opp = opp_zero;
	VALIDATE(opp);
	opp->name = &str_empty[0];
	opp->clanname = &str_empty[0];
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
	static EVENT_DATA event_zero;
	EVENT_DATA *event;

	if (event_free == NULL)
		event = alloc_perm2(sizeof(*event), "War Event");
	else {
		event = event_free;
		event_free = event_free->next;
	}

	*event = event_zero;
	VALIDATE(event);
	event->astr = &str_empty[0];
	event->bstr = &str_empty[0];
	event->time = (time_t) 0;
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
	static MERC_DATA merc_zero;
	MERC_DATA *merc;

	if (merc_free == NULL)
		merc = alloc_perm2(sizeof(*merc), "Merc");
	else {
		merc = merc_free;
		merc_free = merc_free->next;
	}

	*merc = merc_zero;
	VALIDATE(merc);
	merc->name = &str_empty[0];
	merc->employer = &str_empty[0];
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
	static OFFER_DATA offer_zero;
	OFFER_DATA *offer;

	if (offer_free == NULL)
		offer = alloc_perm2(sizeof(*offer), "Merc Offer");
	else {
		offer = offer_free;
		offer_free = offer_free->next;
	}

	*offer = offer_zero;
	VALIDATE(offer);
	offer->name = &str_empty[0];
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
	static DUEL_DATA duel_zero;
	DUEL_DATA *duel;

	if (duel_free == NULL)
		duel = alloc_perm2(sizeof(*duel), "Duel");
	else {
		duel = duel_free;
		duel_free = duel_free->next;
	}

	*duel = duel_zero;
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

/* buffer sizes */
const long buf_size[MAX_BUF_LIST] = {
	16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size(long val)
{
	int i;

	for (i = 0; i < MAX_BUF_LIST; i++)
		if (buf_size[i] >= val)
			return buf_size[i];

	return -1;
}

BUFFER *new_buf()
{
	BUFFER *buffer;

	if (buf_free == NULL)
		buffer = alloc_perm2(sizeof(*buffer), "Buffer");
	else {
		buffer = buf_free;
		buf_free = buf_free->next;
	}

	buffer->next        = NULL;
	buffer->state       = BUFFER_SAFE;
	buffer->size        = get_size(BASE_BUF);
	buffer->string      = alloc_mem(buffer->size);
	buffer->string[0]   = '\0';
	VALIDATE(buffer);
	return buffer;
}

BUFFER *new_buf_size(long size)
{
	BUFFER *buffer;

	if (buf_free == NULL)
		buffer = alloc_perm2(sizeof(*buffer), "Buffer");
	else {
		buffer = buf_free;
		buf_free = buf_free->next;
	}

	buffer->next        = NULL;
	buffer->state       = BUFFER_SAFE;
	buffer->size        = get_size(size);

	if (buffer->size == -1) {
		bug("new_buf: buffer size %d too large.", size);
		exit(1);
	}

	buffer->string      = alloc_mem(buffer->size);
	buffer->string[0]   = '\0';
	VALIDATE(buffer);
	return buffer;
}


void free_buf(BUFFER *buffer)
{
	if (!IS_VALID(buffer))
		return;

	free_mem(buffer->string, buffer->size);
	buffer->string = NULL;
	buffer->size   = 0;
	buffer->state  = BUFFER_FREED;
	INVALIDATE(buffer);
	buffer->next  = buf_free;
	buf_free      = buffer;
}


bool add_buf(BUFFER *buffer, char *string)
{
	int len;
	char *oldstr;
	long oldsize;
	oldstr = buffer->string;
	oldsize = buffer->size;

	if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
		return FALSE;

	len = strlen(buffer->string) + strlen(string) + 1;

	while (len >= buffer->size) { /* increase the buffer size */
		buffer->size    = get_size(buffer->size + 1);
		{
			if (buffer->size == -1) { /* overflow */
				buffer->size = oldsize;
				buffer->state = BUFFER_OVERFLOW;
				bug("buffer overflow past size %d", buffer->size);
				log_string("where: ");
				log_string(dv_where);

				if (dv_char != NULL && dv_char->name != NULL) {
					log_string("char: ");
					log_string(dv_char->name);
				}

				log_string("command:");
				log_string(dv_command);
				log_string("string:");
				log_string(string);
				return FALSE;
			}
		}
	}

	if (buffer->size != oldsize) {
		buffer->string  = alloc_mem(buffer->size);
		strcpy(buffer->string, oldstr);
		free_mem(oldstr, oldsize);
	}

	strcat(buffer->string, string);
	return TRUE;
}


void clear_buf(BUFFER *buffer)
{
	buffer->string[0] = '\0';
	buffer->state     = BUFFER_SAFE;
	return;
}


char *buf_string(BUFFER *buffer)
{
	return buffer->string;
}

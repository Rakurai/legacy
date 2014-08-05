/***************************************************************************
* Duel Module                                                              *
* 2002 Jason Anderson, proprietary for Legacy.                             *
***************************************************************************/

#include "merc.h"
#include "recycle.h"

#define ARENA_DIR       "../misc/"
#define ARENA_FILE      "arena.txt"

void remove_duel(DUEL_DATA *c);
ROOM_INDEX_DATA *get_random_arena_room(ARENA_DATA *arena, int notvnum);
void duel_announce(char *buf, DUEL_DATA *duel);
void clear_arena(ARENA_DATA *arena);

struct duel_data *duel_table_head;
struct duel_data *duel_table_tail;
struct arena_data *arena_table_head;
struct arena_data *arena_table_tail;

/* called every second */
void duel_update()
{
	char buf[MSL];
	DUEL_DATA *c, *c_next;
	ROOM_INDEX_DATA *room;
	c = duel_table_head->next;

	while (c != duel_table_tail) {
		if (c->accept_timer > 0) {
			if (--c->accept_timer == 0) {
				sprintf(buf, "%s seems to have fallen asleep, %s had better find somebody awake to duel!",
				        c->defender->name, c->challenger->name);
				duel_announce(buf, c);
				ptc(c->defender, "{P[{RDUEL{P] {WYou didn't respond to %s's challenge, it has been withdrawn.{x\n",
				    c->challenger->name);
				ptc(c->challenger, "{P[{RDUEL{P] {W%s didn't respond to your challenge, it has been withdrawn.{x\n",
				    c->defender->name);
				c_next = c->next;
				remove_duel(c);
				c = c_next;
				continue;
			}
		}

		if (c->prep_timer > 0) {
			if (--c->prep_timer == 0) {
				CHAR_DATA *wch;

				for (wch = c->arena->viewroom->people; wch != NULL; wch = wch->next_in_room)
					stc("{P[{RDUEL{P] {WThe duel has begun!{x\n", wch);

				room = get_random_arena_room(c->arena, 0);
				char_from_room(c->challenger);
				char_to_room(c->challenger, room);
				do_look(c->challenger, "auto");
				room = get_random_arena_room(c->arena, room->vnum);
				char_from_room(c->defender);
				char_to_room(c->defender, room);
				do_look(c->defender, "auto");
				stc("\n{P[{RDUEL{P] {WThe duel has begun.  Luck be with ye!{x\n", c->challenger);
				stc("\n{P[{RDUEL{P] {WThe duel has begun.  Luck be with ye!{x\n", c->defender);
			}
		}

		c = c->next;
	}
}

void load_arena_table()
{
	FILE *fp;
	int i, maxArenas;
	ARENA_DATA *new_arena;

	if ((fp = fopen(ARENA_DIR ARENA_FILE, "r")) != NULL) {
		fscanf(fp, "%d\n", &maxArenas);
		arena_table_head                = alloc_mem(sizeof(ARENA_DATA));
		arena_table_tail                = alloc_mem(sizeof(ARENA_DATA));
		arena_table_head->next          = arena_table_tail;
		arena_table_tail->previous      = arena_table_head;
		duel_table_head         = alloc_mem(sizeof(DUEL_DATA));
		duel_table_tail         = alloc_mem(sizeof(DUEL_DATA));
		duel_table_head->next   = duel_table_tail;
		duel_table_tail->previous       = duel_table_head;

		for (i = 0; i < maxArenas; i++) {
			new_arena = alloc_mem(sizeof(ARENA_DATA));
			new_arena->keyword      = str_dup(fread_string(fp));
			new_arena->name         = str_dup(fread_string(fp));
			new_arena->desc         = str_dup(fread_string(fp));
			new_arena->minvnum      = atoi(fread_string(fp));
			new_arena->maxvnum      = atoi(fread_string(fp));
			new_arena->chalprep     = get_room_index(atoi(fread_string(fp)));
			new_arena->defprep      = get_room_index(atoi(fread_string(fp)));
			new_arena->viewroom     = get_room_index(atoi(fread_string(fp)));

			if (new_arena->chalprep == NULL
			    || new_arena->defprep  == NULL
			    || new_arena->viewroom == NULL) {
				printf("Bad arena room!");
				exit(1);
			}

			new_arena->previous             = arena_table_tail->previous;
			new_arena->previous->next       = new_arena;
			new_arena->next                 = arena_table_tail;
			arena_table_tail->previous      = new_arena;
		}

		fclose(fp);
	}
	else
		bug("Could not open " ARENA_FILE " for reading!", 0);
}

void append_duel(DUEL_DATA *c)
{
	c->previous                     = duel_table_tail->previous;
	c->previous->next               = c;
	c->next                         = duel_table_tail;
	duel_table_tail->previous       = c;
	c->challenger->pcdata->duel     = c;
	c->defender->pcdata->duel       = c;
}

void remove_duel(DUEL_DATA *c)
{
	CHAR_DATA *ch;
	c->previous->next       = c->next;
	c->next->previous       = c->previous;

	for (ch = char_list; ch != NULL; ch = ch->next)
		if (!IS_NPC(ch) && ch->pcdata->duel == c)
			ch->pcdata->duel = NULL;

	clear_arena(c->arena);
	free_duel(c);
}

void duel_announce(char *buf, DUEL_DATA *duel)
{
	char buffer[MSL];
	DESCRIPTOR_DATA *d;
	sprintf(buffer, "{P[{RDUEL{P] {W%s{x\n", buf);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (IS_PLAYING(d)
		    && d->character != duel->challenger
		    && d->character != duel->defender
		    && !IS_SET(d->character->comm, COMM_NOANNOUNCE)
		    && !IS_SET(d->character->comm, COMM_QUIET))
			stc(buffer, d->character);
}

bool char_in_dprep_room(CHAR_DATA *ch)
{
	ARENA_DATA *arena = arena_table_head->next;

	if (ch->in_room == NULL)
		return FALSE;

	while (arena != arena_table_tail) {
		if (ch->in_room == arena->chalprep
		    || ch->in_room == arena->defprep)
			return TRUE;

		arena = arena->next;
	}

	return FALSE;
}

bool char_in_darena_room(CHAR_DATA *ch)
{
	ARENA_DATA *arena = arena_table_head->next;

	if (ch->in_room == NULL)
		return FALSE;

	while (arena != arena_table_tail) {
		if (ch->in_room->vnum >= arena->minvnum
		    && ch->in_room->vnum <= arena->maxvnum)
			return TRUE;

		arena = arena->next;
	}

	return FALSE;
}

bool char_in_duel_room(CHAR_DATA *ch)
{
	ARENA_DATA *arena = arena_table_head->next;

	if (ch->in_room == NULL)
		return FALSE;

	while (arena != arena_table_tail) {
		if (ch->in_room == arena->chalprep
		    || ch->in_room == arena->defprep
		    || (ch->in_room->vnum >= arena->minvnum
		        && ch->in_room->vnum <= arena->maxvnum))
			return TRUE;

		arena = arena->next;
	}

	return FALSE;
}

bool char_in_dprep(CHAR_DATA *ch)
{
	DUEL_DATA *duel;

	if ((duel = get_duel(ch)) == NULL)
		return FALSE;

	if (duel->prep_timer != 0)
		return TRUE;

	return FALSE;
}

bool char_in_darena(CHAR_DATA *ch)
{
	DUEL_DATA *duel;

	if ((duel = get_duel(ch)) == NULL)
		return FALSE;

	if (duel->accept_timer == 0 && duel->prep_timer == 0)
		return TRUE;

	return FALSE;
}

bool char_in_duel(CHAR_DATA *ch)
{
	DUEL_DATA *duel;

	if ((duel = get_duel(ch)) == NULL)
		return FALSE;

	if (duel->accept_timer == 0)
		return TRUE;

	return FALSE;
}

/* return a player's duel, massive debugging at lowest level */
DUEL_DATA *get_duel(CHAR_DATA *ch)
{
	DUEL_DATA *duel;
	CHAR_DATA *opp = NULL;
	bool cgr = FALSE;

	if (IS_NPC(ch) || ch->in_room == NULL || ch->pcdata == NULL || ch->pcdata->duel == NULL)
		return NULL;

	duel = ch->pcdata->duel;

	if (duel->challenger == ch) {
		cgr = TRUE;

		if ((opp = duel->defender) == NULL) {
			bug("get_duel: defender is NULL", 0);
			goto bombout;
		}
	}
	else if (duel->defender == ch) {
		cgr = FALSE;

		if ((opp = duel->challenger) == NULL) {
			bug("get_duel: challenger is NULL", 0);
			goto bombout;
		}
	}
	else {
		bug("get_duel: ch not in duel", 0);
		goto bombout;
	}

	if (opp == ch) {
		bug("get_duel: opp == ch", 0);
		goto bombout;
	}

	if (opp->pcdata->duel == NULL) {
		bug("get_duel: opp->pcdata->duel == NULL", 0);
		goto bombout;
	}

	if (opp->pcdata->duel != duel) {
		bug("get_duel: opp->pcdata->duel != duel", 0);
		goto bombout;
	}

	if (opp->in_room == NULL) {
		bug("get_duel: opp->in_room == NULL", 0);
		goto bombout;
	}

	if (duel->arena == NULL) {
		bug("get_duel: arena is NULL", 0);
		goto bombout;
	}

	if (duel->accept_timer == 0) {
		if (duel->prep_timer == 0) {
			if (ch->in_room->vnum > duel->arena->maxvnum
			    || ch->in_room->vnum < duel->arena->minvnum) {
				bug("get_duel: timers 0, ch not in arena", 0);
				goto bombout;
			}

			if (opp->in_room->vnum > duel->arena->maxvnum
			    || opp->in_room->vnum < duel->arena->minvnum) {
				bug("get_duel: timers 0, opp not in arena", 0);
				goto bombout;
			}
		}
		else {
			if (cgr) {
				if (ch->in_room != duel->arena->chalprep) {
					bug("get_duel: accept 0, ch not in chalprep", 0);
					goto bombout;
				}

				if (opp->in_room != duel->arena->defprep) {
					bug("get_duel: accept 0, opp not in defprep", 0);
					goto bombout;
				}
			}
			else {
				if (ch->in_room != duel->arena->defprep) {
					bug("get_duel: accept 0, ch not in defprep", 0);
					goto bombout;
				}

				if (opp->in_room != duel->arena->chalprep) {
					bug("get_duel: accept 0, opp not in chalprep", 0);
					goto bombout;
				}
			}
		}
	}

	return duel;
bombout:
	remove_duel(duel);
	return NULL;
}

ARENA_DATA *get_random_arena()
{
	ARENA_DATA *arena;
	int count = 0, number;

	for (arena = arena_table_head->next; arena != arena_table_tail; arena = arena->next)
		count++;

	number = number_range(1, count);
	count = 0;

	for (arena = arena_table_head->next; arena != arena_table_tail; arena = arena->next) {
		count++;

		if (number == count)
			break;
	}

	/* no arenas? */
	if (arena == arena_table_tail)
		return NULL;

	return arena;
}

ROOM_INDEX_DATA *get_random_arena_room(ARENA_DATA *arena, int notvnum)
{
	ROOM_INDEX_DATA *room;

	do {
		room = get_room_index(number_range(arena->minvnum, arena->maxvnum));
	}
	while (room == NULL || room->vnum == notvnum);

	return room;
}

void view_room_hpbar(CHAR_DATA *ch)
{
	char chalblock[MSL], defblock[MSL], line[MSL];
	DUEL_DATA *duel;
	CHAR_DATA *chal, *def, *vch;
	int i, chalpct, defpct;

	if ((duel = get_duel(ch)) == NULL)
		return;

	if (duel->accept_timer != 0 || duel->prep_timer != 0)
		return;

	/* they'll hopefully be fighting eachother, but there's other possibilities.
	   so, if they're not fighting eachother, we still want to print the bar,
	   but only once */
	if (duel->defender == ch && duel->challenger->fighting)
		return;

	chal = duel->challenger;
	def  = duel->defender;
	chalpct = URANGE(1, 10 * chal->hit / chal->max_hit, 10);
	defpct  = URANGE(1, 10 * def->hit  / def->max_hit,  10);
	sprintf(chalblock, "{C%s{C [", chal->name);

	if (chalpct <= 3)       strcat(chalblock, "{P");
	else                    strcat(chalblock, "{G");

	for (i = 1; i < 11; i++) {
		if (chalpct == i) {
			strcat(chalblock, "{Y*");

			if (chalpct <= 3)       strcat(chalblock, "{R");
			else                    strcat(chalblock, "{H");

			continue;
		}

		strcat(chalblock, "*");
	}

	strcat(chalblock, "{C]");
	sprintf(defblock, "{C[");

	if (defpct <= 3)        strcat(defblock, "{P");
	else                    strcat(defblock, "{G");

	for (i = 1; i < 11; i++) {
		if (defpct == i) {
			strcat(defblock, "{Y*");

			if (defpct <= 3)        strcat(defblock, "{R");
			else                    strcat(defblock, "{H");

			continue;
		}

		strcat(defblock, "*");
	}

	strcat(defblock, "{C] ");
	strcat(defblock, def->name);
	sprintf(line, "%30s %-30s{x\n", chalblock, defblock);

	for (vch = duel->arena->viewroom->people; vch != NULL; vch = vch->next_in_room)
		stc(line, vch);
}

void clear_arena(ARENA_DATA *arena)
{
	ROOM_INDEX_DATA *room;
	CHAR_DATA *wch;
	int i;

	for (i = arena->minvnum; i != arena->maxvnum + 1; i++)
		if ((room = get_room_index(i)) != NULL && room->people)
			for (wch = room->people; wch != NULL; wch = wch->next_in_room)
				if (!IS_IMMORTAL(wch))
					extract_char(wch, !IS_NPC(wch));

	if (arena->chalprep->people)
		for (wch = arena->chalprep->people; wch != NULL; wch = wch->next_in_room)
			if (!IS_IMMORTAL(wch))
				extract_char(wch, !IS_NPC(wch));

	if (arena->defprep->people)
		for (wch = arena->defprep->people; wch != NULL; wch = wch->next_in_room)
			if (!IS_IMMORTAL(wch))
				extract_char(wch, !IS_NPC(wch));
}

void duel_kill(CHAR_DATA *victim)
{
	char buf[MSL];
	CHAR_DATA *ch, *wch;
	DUEL_DATA *duel;
	ROOM_INDEX_DATA *room;
	int room_vnum;
	duel = get_duel(victim);

	if (duel->challenger == victim)
		ch = duel->defender;
	else
		ch = duel->challenger;

	char_from_room(ch);
	char_from_room(victim);

	if (ch->clan) {
		char_to_room(ch, get_room_index(ch->clan->hall));
		stc("You find yourself back in your clanhall.\n", ch);
	}
	else {
		char_to_room(ch, get_room_index(ROOM_VNUM_ALTAR));
		stc("You find yourself at the altar of Mota.\n", ch);
	}

	if (victim->clan) {
		char_to_room(victim, get_room_index(victim->clan->hall));
		stc("You find yourself back in your clanhall.\n", victim);
	}
	else {
		char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));
		stc("You find yourself at the altar of Mota.\n", victim);
	}

	/* go get their pets */
	for (room_vnum = duel->arena->minvnum; room_vnum != duel->arena->maxvnum + 1; room_vnum++) {
		room = get_room_index(room_vnum);

		if (! room)
			bug("Error with get_room_index() in duel_kill() in duel.c.", 0);

		if (room->people)
			for (wch = room->people; wch != NULL; wch = wch->next)
				if (wch->master == ch || wch->master == victim) {
					char_from_room(wch);
					char_to_room(wch, wch->master->in_room);
				}
	}

	if (char_opponents(ch, victim))
		sprintf(buf, "%s {Whas won this victory, may %s {Wlead them to another.",
		        ch->clan->clanname, ch->name);
	else
		sprintf(buf, "%s {Whas emerged victorious!  Better luck next time, %s{W.",
		        ch->name, victim->name);

	duel_announce(buf, duel);
	remove_duel(duel);
	save_char_obj(ch);
	save_char_obj(victim);
}

void prepare_char(CHAR_DATA *ch, DUEL_DATA *duel)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int loc;
	extern void affect_modify args((CHAR_DATA *, AFFECT_DATA *, bool));
	char_from_room(ch);

	if (duel->challenger == ch)
		char_to_room(ch, duel->arena->chalprep);
	else
		char_to_room(ch, duel->arena->defprep);

	while (ch->affected)
		affect_remove(ch, ch->affected);

	ch->affected_by = race_table[ch->race].aff;

	for (loc = 0; loc < MAX_WEAR; loc++) {
		if ((obj = get_eq_char(ch, loc)) == NULL)
			continue;

		if (!obj->enchanted)
			for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
				if (paf->where == TO_AFFECTS && paf->location != APPLY_SPELL_AFFECT)
					affect_modify(ch, paf, TRUE);

		for (paf = obj->affected; paf != NULL; paf = paf->next)
			if (paf->where == TO_AFFECTS)
				affect_modify(ch, paf, TRUE);
	}

	ch->hit  = ch->max_hit;
	ch->mana = ch->max_mana;
	ch->stam = ch->max_stam;
	do_look(ch, "auto");
}

void do_duel(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[MSL];
	DUEL_DATA *duel;
	CHAR_DATA *victim = NULL;
	ARENA_DATA *arena;

	if (IS_NPC(ch)) {
		stc("You have no need to challenge players.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (!str_prefix1(arg1, "arena")) {
		if (arg2[0] == '\0') {
			stc("Keyword:        Name:\n", ch);

			for (arena = arena_table_head->next; arena != arena_table_tail; arena = arena->next)
				ptc(ch, "%-16s%s\n", arena->keyword, arena->name);

			stc("\nType 'duel arena <keyword>' for more information.\n", ch);
			return;
		}

		for (arena = arena_table_head->next; arena != arena_table_tail; arena = arena->next)
			if (!str_prefix1(arg2, arena->keyword))
				break;

		if (arena == arena_table_tail) {
			stc("Arena not found.  Type 'duel arena' to see the choices.\n", ch);
			return;
		}

		ptc(ch, "%s\n\n%s\n", arena->name, arena->desc);
		return;
	}

	if (!str_prefix1(arg1, "decline")) {
		if ((duel = get_duel(ch)) == NULL || duel->defender != ch) {
			stc("No duel has been issued to you.\n", ch);
			return;
		}

		if (duel->accept_timer == 0) {
			stc("Too late, get ready to fight!\n", ch);
			return;
		}

		if (char_opponents(ch, duel->challenger))
			sprintf(buf, "%s {Whas declined %s{W's challenge, %s {Whad better find another fighter!",
			        ch->name, duel->challenger->name, ch->clan->clanname);
		else
			sprintf(buf, "%s {Whas declined %s{W's challenge, what a wuss!",
			        ch->name, duel->challenger->name);

		duel_announce(buf, duel);
		ptc(ch, "{P[{RDUEL{P] {WYou decline %s{W's challenge.{x\n", duel->challenger->name);
		ptc(duel->challenger, "{P[{RDUEL{P] {W%s {Wdeclines your challenge.{x\n", ch->name);
		remove_duel(duel);
		return;
	}

	if (!str_prefix1(arg1, "ignore")) {
		if (IS_IMMORTAL(ch)) {
			stc("Immortals cannot duel.\n", ch);
			return;
		}

		if (get_duel(ch)) {
			stc("You have a duel in progress, deal with it first.\n", ch);
			return;
		}

		if (IS_SET(ch->pcdata->plr, PLR_DUEL_IGNORE)) {
			REMOVE_BIT(ch->pcdata->plr, PLR_DUEL_IGNORE);
			stc("Others can challenge you now.\n", ch);
		}
		else {
			SET_BIT(ch->pcdata->plr, PLR_DUEL_IGNORE);
			stc("You no longer fight in duels.\n", ch);
		}

		return;
	}

	if (arg1[0] != '\0' && ch->in_room != NULL && ch->in_room->vnum == 1212) {
		stc("Put your nose back in the corner, you don't need to duel.\n", ch);
		return;
	}

	if (!str_prefix1(arg1, "issue")) {
		if (ch->level < 10 && !IS_REMORT(ch)) {
			stc("You cannot duel until you are level 10.\n", ch);
			return;
		}

		if (arg2[0] == '\0') {
			stc("Syntax: duel issue <victim> <arena name (optional)>\n", ch);
			return;
		}

		if (IS_IMMORTAL(ch)) {
			stc("Immortals cannot duel.\n", ch);
			return;
		}

		if (get_duel(ch)) {
			stc("You are already involved in a duel, deal with it first.\n", ch);
			return;
		}

		if ((victim = get_player_world(ch, arg2, VIS_PLR)) == NULL) {
			stc("You see no one by that name around.\n", ch);
			return;
		}

		if (ch == victim) {
			stc("You slap yourself around a bit.\n", ch);
			return;
		}

		if (IS_IMMORTAL(victim)) {
			stc("Immortals cannot duel.\n", ch);
			return;
		}

		if (victim->level < 10 && !IS_REMORT(victim)) {
			stc("You cannot duel with newbies.\n", ch);
			return;
		}

		if (IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)
		    || IS_SET(victim->comm, COMM_AFK)) {
			stc("They are not with us at present, wait until they return.\n", ch);
			return;
		}

		if (IS_SET(victim->pcdata->plr, PLR_DUEL_IGNORE)) {
			stc("They are not accepting duels.\n", ch);
			return;
		}

		if (char_opponents(ch, victim)
		    && ((ch->level - victim->level) > 10
		        || (victim->level - ch->level) > 10)) {
			stc("You must be within ten levels of your opponent to duel in war.\n", ch);
			return;
		}

		if (get_duel(victim)) {
			stc("They are already involved in a duel.\n", ch);
			return;
		}

		if (arg3[0] == '\0') {
			if ((arena = get_random_arena()) == NULL) {
				stc("Sorry, there are no arenas right now.\n", ch);
				return;
			}
		}
		else {
			for (arena = arena_table_head->next; arena != arena_table_tail; arena = arena->next)
				if (is_name(arg3, arena->keyword))
					break;

			if (arena == arena_table_tail) {
				stc("That is not an arena.\n", ch);
				return;
			}
		}

		duel = new_duel();
		duel->challenger = ch;
		duel->defender = victim;
		duel->arena = arena;
		duel->accept_timer = 600;
		duel->prep_timer = 0;
		append_duel(duel);
		ptc(ch, "{P[{RDUEL{P] {WYou challenge %s {Wto a duel!{x\n", victim->name);
		ptc(victim, "{P[{RDUEL{P] {W%s {Whas challenged you to a duel!{x\n", ch->name);

		if (char_opponents(ch, victim))
			sprintf(buf, "%s {Whas challenged %s {Wto a duel for the honor of %s!",
			        ch->name, victim->name, victim->clan->clanname);
		else
			sprintf(buf, "%s {Whas challenged %s {Wto a duel!\n",
			        ch->name, victim->name);

		duel_announce(buf, duel);
		return;
	}

	if (!str_prefix1(arg1, "accept")) {
		if (IS_IMMORTAL(ch)) {
			stc("Immortals cannot duel.\n", ch);
			return;
		}

		duel = duel_table_head->next;

		while (duel != duel_table_tail) {
			if (duel->accept_timer == 0) {
				if (ch->pcdata->duel == duel)
					stc("You have already accepted.\n", ch);
				else
					stc("A duel is already in progress.\n", ch);

				return;
			}

			duel = duel->next;
		}

		if ((duel = get_duel(ch)) == NULL || duel->defender != ch) {
			stc("You have not been challenged to a duel.\n", ch);
			return;
		}

		clear_arena(duel->arena);
		victim = duel->challenger;
		prepare_char(ch, duel);
		prepare_char(victim, duel);
		duel->accept_timer = 0;
		duel->prep_timer = 60;
		stc("{P[{RDUEL{P] {WYou accept the challenge, prepare for combat!{x\n", ch);
		ptc(victim, "{P[{RDUEL{P] {W%s {Waccepts your challenge, prepare for combat!{x\n", ch->name);
		sprintf(buf, "%s {Whas accepted %s{W's challenge, the duel is going to start!",
		        ch->name, victim->name);
		duel_announce(buf, duel);
		return;
	}

	if (!str_prefix1(arg1, "view")) {
		duel = duel_table_head->next;

		while (duel != duel_table_tail) {
			if (duel->accept_timer == 0)
				break;

			duel = duel->next;
		}

		if (duel == duel_table_tail) {
			stc("No duels have commenced.\n", ch);
			return;
		}

		if (ch->pcdata->duel == duel) {
			stc("You can't view your own duel!\n", ch);
			return;
		}

		if (ch->fighting) {
			stc("Deal with this fight first!\n", ch);
			return;
		}

		if (ch->in_room == NULL || ch->in_room == duel->arena->viewroom) {
			stc("You're already there!\n", ch);
			return;
		}

		act("$n leaves to watch the duel!", ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, duel->arena->viewroom);
		act("$n has arrived to watch the duel!", ch, 0, 0, TO_ROOM);
		do_look(ch, "auto");
		return;
	}

	stc("Syntax:\n", ch);
	stc("  duel issue <opponent> <arena (optional)>\n", ch);
	stc("  duel accept\n", ch);
	stc("  duel decline\n", ch);
	stc("  duel view\n", ch);
	stc("  duel ignore\n", ch);
	stc("  duel arena\n", ch);
	stc("  duel arena <arena name>\n", ch);
}

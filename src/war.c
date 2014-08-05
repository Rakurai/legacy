/***************************************************************************
* War Module                                                               *
* 2002 Jason Anderson, proprietary for Legacy.                             *
***************************************************************************/

#include "merc.h"
#include "recycle.h"

#define WAR_DIR         "../war/"
#define EVENT_DIR       "../war/events/"
#define WAR_FILE        "war.txt"
#define EVENT_TMP       "eventtmp"

WAR_DATA *get_same_war(CLAN_DATA *clanA, CLAN_DATA *clanB);
void war_win(WAR_DATA *war, CHAR_DATA *ch);
int get_war_index(WAR_DATA *war);
void rec_event(WAR_DATA *war, int type, char *astr, char *bstr, int number);

struct war_data *war_table_head;
struct war_data *war_table_tail;

void append_war(WAR_DATA *war)
{
	war->previous           = war_table_tail->previous;
	war->previous->next     = war;
	war->next               = war_table_tail;
	war_table_tail->previous = war;
}

void load_war_events()
{
	char strsave[MIL];
	char *tmp_string;
	FILE *fp;
	WAR_DATA *war;
	EVENT_DATA *event, *n_event;
	war = war_table_head->next;

	while (war != war_table_tail) {
		sprintf(strsave, "%sWarEvents%d", EVENT_DIR, get_war_index(war));

		if ((fp = fopen(strsave, "r")) != NULL) {
			for (; ;) {
				if (!str_cmp(fread_word(fp), "END"))
					break;

				n_event         = new_event();
				n_event->type   = atoi(fread_string(fp));
				n_event->astr   = str_dup(fread_string(fp));
				n_event->bstr   = str_dup(fread_string(fp));
				n_event->number = atoi(fread_string(fp));
				tmp_string      = fread_string(fp);
				n_event->time   = dizzy_scantime(tmp_string);
				free_string(tmp_string);

				if (war->events == NULL) /* first event? */
					war->events = n_event;
				else
					for (event = war->events; event != NULL; event = event->next)
						if (event->next == NULL) {
							event->next = n_event;
							break;
						}
			}

			fclose(fp);
		}
		else
			bug("Could not open event file for reading!", 0);

		war = war->next;
	}
}

void save_war_events()
{
	char strsave[MIL];
	FILE *fp;
	EVENT_DATA *event;
	WAR_DATA *war;
	war = war_table_head->next;

	while (war != war_table_tail) {
		sprintf(strsave, "%sWarEvents%d", EVENT_DIR, get_war_index(war));

		if ((fp = fopen(EVENT_TMP, "w")) != NULL) {
			event = war->events;

			while (event != NULL) {
				fprintf(fp, "NOTEND\n");
				fprintf(fp, "%d~\n", event->type);
				fprintf(fp, "%s~\n", event->astr);
				fprintf(fp, "%s~\n", event->bstr);
				fprintf(fp, "%d~\n", event->number);
				fprintf(fp, "%s~\n", dizzy_ctime(&event->time));
				event = event->next;
			}

			fprintf(fp, "END\n");
			fclose(fp);
			rename(EVENT_TMP, strsave);
		}
		else
			bug("Could not open " EVENT_TMP " for writing!", 0);

		war = war->next;
	}
}

void load_war_table()
{
	FILE *fp;
	WAR_DATA *war;
	int i, count = 0;
	war_table_head = new_war();
	war_table_tail = new_war();
	war_table_head->next            = war_table_tail;
	war_table_tail->previous        = war_table_head;

	if ((fp = fopen(WAR_DIR WAR_FILE, "r")) != NULL) {
		for (; ;) {
			if (!str_cmp(fread_word(fp), "END"))
				break;

			war = new_war();

			for (i = 0; i < 4; i++) {
				war->chal[i]->name              = str_dup(fread_string(fp));
				war->chal[i]->clanname          = str_dup(fread_string(fp));
				war->chal[i]->inwar             = atoi(fread_string(fp));
				war->chal[i]->start_score       = atoi(fread_string(fp));
				war->chal[i]->final_score       = atoi(fread_string(fp));
			}

			for (i = 0; i < 4; i++) {
				war->def[i]->name               = str_dup(fread_string(fp));
				war->def[i]->clanname           = str_dup(fread_string(fp));
				war->def[i]->inwar              = atoi(fread_string(fp));
				war->def[i]->start_score        = atoi(fread_string(fp));
				war->def[i]->final_score        = atoi(fread_string(fp));
			}

			war->ongoing = atoi(fread_string(fp));
			append_war(war);
			count++;
		}

		fclose(fp);
	}
	else
		bug("Could not open " WAR_FILE " for reading!", 0);
}

void save_war_table()
{
	FILE *fp;
	WAR_DATA *war;
	int i;

	if ((fp = fopen(WAR_DIR WAR_FILE, "w")) != NULL) {
		war = war_table_head->next;

		while (war != war_table_tail) {
			fprintf(fp, "NOTEND\n");

			for (i = 0; i < 4; i++) {
				fprintf(fp, "%s~\n", war->chal[i]->name);
				fprintf(fp, "%s~\n", war->chal[i]->clanname);
				fprintf(fp, "%d~\n", war->chal[i]->inwar);
				fprintf(fp, "%d~\n", war->chal[i]->start_score);
				fprintf(fp, "%d~\n", war->chal[i]->final_score);
			}

			for (i = 0; i < 4; i++) {
				fprintf(fp, "%s~\n", war->def[i]->name);
				fprintf(fp, "%s~\n", war->def[i]->clanname);
				fprintf(fp, "%d~\n", war->def[i]->inwar);
				fprintf(fp, "%d~\n", war->def[i]->start_score);
				fprintf(fp, "%d~\n", war->def[i]->final_score);
			}

			fprintf(fp, "%d~\n", war->ongoing);
			war = war->next;
		}

		fprintf(fp, "END\n");
		fclose(fp);
	}
	else
		bug("Could not open " WAR_FILE " for writing!", 0);
}

void fix_war(WAR_DATA *war)
{
	WAR_DATA *fixed_war = new_war();
	int i, c = 0, d = 0;

	for (i = 0; i < 4; i++) {
		if (war->chal[i]->name[0] != '\0') {
			fixed_war->chal[c]->name        = war->chal[i]->name;
			fixed_war->chal[c]->clanname    = war->chal[i]->clanname;
			fixed_war->chal[c]->inwar       = war->chal[i]->inwar;
			fixed_war->chal[c]->start_score = war->chal[i]->start_score;
			fixed_war->chal[c]->final_score = war->chal[i]->final_score;
			c++;
		}

		if (war->def[i]->name[0] != '\0') {
			fixed_war->def[d]->name         = war->def[i]->name;
			fixed_war->def[d]->clanname     = war->def[i]->clanname;
			fixed_war->def[d]->inwar        = war->def[i]->inwar;
			fixed_war->def[d]->start_score  = war->def[i]->start_score;
			fixed_war->def[d]->final_score  = war->def[i]->final_score;
			d++;
		}
	}

	fixed_war->events   = war->events;
	fixed_war->ongoing  = war->ongoing;
	fixed_war->previous = war->previous;
	fixed_war->next     = war->next;
	fixed_war->previous->next = fixed_war;
	fixed_war->next->previous = fixed_war;
	free_war(war);
}

bool clan_in_war(CLAN_DATA *clan, WAR_DATA *war, bool onlycurrent)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (!str_cmp(clan->name, war->chal[i]->name)) {
			if (onlycurrent) {
				if (war->chal[i]->inwar)
					return TRUE;
			}
			else
				return TRUE;
		}

		if (!str_cmp(clan->name, war->def[i]->name)) {
			if (onlycurrent) {
				if (war->def[i]->inwar)
					return TRUE;
			}
			else
				return TRUE;
		}
	}

	return FALSE;
}

bool clan_at_war(CLAN_DATA *clan)
{
	WAR_DATA *war;
	war = war_table_head->next;

	while (war != war_table_tail) {
		if (war->ongoing)
			if (clan_in_war(clan, war, TRUE))
				return TRUE;

		war = war->next;
	}

	return FALSE;
}

bool clan_is_challenger(CLAN_DATA *clan, WAR_DATA *war)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (!str_cmp(clan->name, war->chal[i]->name) && war->chal[i]->inwar)
			return TRUE;
		else if (!str_cmp(clan->name, war->def[i]->name) && war->def[i]->inwar)
			return FALSE;
	}

	bug("clan_is_challenger: clan not in war", 0);
	return FALSE;
}

bool clan_opponents(CLAN_DATA *clanA, CLAN_DATA *clanB)
{
	WAR_DATA *war = NULL;

	if ((war = get_same_war(clanA, clanB)))
		if (clan_is_challenger(clanA, war) != clan_is_challenger(clanB, war))
			return TRUE;

	return FALSE;
}

bool char_at_war(CHAR_DATA *ch)
{
	if (IS_NPC(ch))
		return FALSE;

	if (!ch->clan)
		return FALSE;

	if (clan_at_war(ch->clan))
		return TRUE;

	return FALSE;
}

bool char_opponents(CHAR_DATA *charA, CHAR_DATA *charB)
{
	/* checks npc, clan, clan at war */
	if (!char_at_war(charA) || !char_at_war(charB))
		return FALSE;

	if (clan_opponents(charA->clan, charB->clan))
		return TRUE;

	return FALSE;
}

bool war_is_full(WAR_DATA *war, bool challenger)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (challenger) {
			if (war->chal[i]->name[0] == '\0')
				return FALSE;
		}
		else {
			if (war->def[i]->name[0] == '\0')
				return FALSE;
		}
	}

	return TRUE;
}

int get_war_index(WAR_DATA *war)
{
	WAR_DATA *iWar;
	int count = 1;
	iWar = war_table_head->next;

	while (iWar != war_table_tail) {
		if (war == iWar)
			return count;

		iWar = iWar->next;
		count++;
	}

	return -1;
}

WAR_DATA *war_lookup(int number)
{
	WAR_DATA *war;
	int count = 1;
	war = war_table_head->next;

	while (war != war_table_tail) {
		if (count == number)
			return war;

		war = war->next;
		count++;
	}

	return NULL;
}

WAR_DATA *get_war(CLAN_DATA *clan)
{
	WAR_DATA *war;
	war = war_table_head->next;

	while (war != war_table_tail) {
		if (war->ongoing && clan_in_war(clan, war, TRUE))
			return war;

		war = war->next;
	}

	return NULL;
}

WAR_DATA *get_same_war(CLAN_DATA *clanA, CLAN_DATA *clanB)
{
	WAR_DATA *war;
	war = war_table_head->next;

	if (clanA == clanB)
		return NULL;

	while (war != war_table_tail) {
		if (war->ongoing)
			if (clan_in_war(clanA, war, TRUE)
			    && clan_in_war(clanB, war, TRUE))
				return war;

		war = war->next;
	}

	return NULL;
}

void defeat_clan(WAR_DATA *war, CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool chal = FALSE, def = FALSE;
	int i;

	for (i = 0; i < 4; i++) {
		if (!str_cmp(victim->clan->name, war->chal[i]->name)) {
			war->chal[i]->inwar = FALSE;
			war->chal[i]->final_score = 0;
		}

		if (!str_cmp(victim->clan->name, war->def[i]->name)) {
			war->def[i]->inwar = FALSE;
			war->def[i]->final_score = 0;
		}

		if (war->chal[i]->name[0] != '\0' && war->chal[i]->inwar)
			chal = TRUE;

		if (war->def[i]->name[0] != '\0' && war->def[i]->inwar)
			def = TRUE;
	}

	rec_event(war, EVENT_CLAN_DEFEAT, victim->clan->clanname, ch->clan->clanname, 0);

	if (!chal && !def) { /* this shouldn't happen */
		bug("No remaining clans in war!", 0);
		return;
	}

	if (chal && def) /* war is still on */
		return;

	/* no opponents left, war is over */
	war_win(war, ch);
}

void war_power_adjust(CLAN_DATA *vclan, bool surrender)
{
	char buf[MSL];
	CLAN_DATA *tclan;
	WAR_DATA *war;
	EVENT_DATA *event;
	int i = 0, x = 0, dealt = 0, loss, highest, listcount = 0, award, numpower, cp = 0, qploss, qpaward;
	bool found;
	struct conq {
		char   *clanname;
		sh_int  scored;
	};
	struct conq conqlist[MAX_CLAN];
	war = war_table_head;

	/* sort through all ongoing wars */
	while (war->next != war_table_tail) {
		war = war->next;

		if (!war->ongoing)
			continue;

		/* sort through the score adjustments, looking for ones pertaining to the victim */
		for (event = war->events; event != NULL; event = event->next) {
			if (event->type != EVENT_ADJUST_SCORE)
				continue;

			if (!str_cmp(event->astr, vclan->clanname)) {
				/* adjusted in victim's favor */
				dealt += event->number;
				continue;
			}

			if (str_cmp(event->bstr, vclan->clanname))
				continue;

			found = FALSE;

			/* adjusted in other clan's favor, find the clan (if still existing) */
			for (tclan = clan_table_head->next; tclan != clan_table_tail; tclan = tclan->next) {
				if (!clan_in_war(tclan, war, TRUE))
					continue;

				if (!str_cmp(event->astr, tclan->clanname)) {
					/* found em */
					i = 0;

					/* see if they're on our list already */
					while (i < listcount) {
						if (!str_cmp(conqlist[i].clanname, tclan->clanname)) {
							conqlist[i].scored += event->number;
							found = TRUE;
							break;
						}

						i++;
					}

					/* nope, add a new entry */
					if (!found) {
						conqlist[i].clanname = str_dup(tclan->clanname);
						conqlist[i].scored = event->number;
						listcount++;
					}

					/* break to next event */
					break;
				}
			}
		}
	}

	if (listcount < 1)
		return;

	/* use real cp, not curved */
	cp = calc_cp(vclan, FALSE);
	/* calculate how much power the clan will lose */
	loss = UMIN(3 + UMAX((cp / 5), 1), cp);         /* max loss */

	if (!surrender)
		loss = URANGE(1, cp - dealt, loss);

	numpower = cp;
	qploss = surrender ? 0 : vclan->clanqp / 10;
	vclan->clanqp -= qploss;
	vclan->warcpmod -= loss;
	cp -= loss;

	if (!cp) {
		DESCRIPTOR_DATA *d;
		CHAR_DATA *victim;

		for (war = war_table_head->next; war != war_table_tail; war = war->next)
			if (war->ongoing && clan_in_war(vclan, war, TRUE))
				rec_event(war, EVENT_CLAN_WIPEOUT, vclan->clanname, NULL, 0);

		sprintf(buf, "[FYI] %s has been wiped out!", vclan->clanname);

		for (d = descriptor_list; d != NULL; d = d->next) {
			victim = d->original ? d->original : d->character;

			if (IS_PLAYING(d)
			    && !IS_SET(victim->comm, COMM_NOANNOUNCE)
			    && !IS_SET(victim->comm, COMM_QUIET)) {
				new_color(victim, CSLOT_CHAN_ANNOUNCE);
				stc(buf, victim);
				set_color(victim, WHITE, NOBOLD);
			}
		}
	}

	/* sort the list */
	i = 0;

	while (i < listcount) {
		highest = 0;
		x = 0;

		/* find the next highest */
		while (x < listcount) {
			if (conqlist[x].scored > highest)
				highest = conqlist[x].scored;

			x++;
		}

		x = 0;

		while (x < listcount && loss > 0) {
			if (conqlist[x].scored == highest) {
				if (conqlist[x + 1].clanname == NULL) {
					qpaward = qploss;
					award = loss;
				}
				else {
					award = URANGE(1, ((((conqlist[x].scored * 100) / numpower)
					                    * loss) / 100), loss);
					qpaward = URANGE(0, ((((conqlist[x].scored * 100) / numpower)
					                      * qploss) / 100), qploss);
				}

				for (tclan = clan_table_head->next;
				     tclan != clan_table_tail;
				     tclan = tclan->next)
					if (!str_cmp(conqlist[x].clanname, tclan->clanname))
						break;

				for (war = war_table_head->next; war != war_table_tail; war = war->next) {
					if (!war->ongoing || !clan_in_war(vclan, war, TRUE))
						continue;

					rec_event(war, EVENT_ADJUST_POWER, vclan->clanname,
					          tclan->clanname, award);

					if (qpaward > 0)
						rec_event(war, EVENT_ADJUST_CLANQP, vclan->clanname,
						          tclan->clanname, qpaward);
				}

				tclan->warcpmod += award;

				if (qpaward > 0)
					tclan->clanqp += qpaward;

				/*                              if (clan_at_war(tclan))
				                                        tclan->score += award;*/
				loss -= award;
				qploss -= qpaward;
				conqlist[x].scored = 0;
				break;
			}

			x++;
		}

		i++;
	}
}

void war_score_adjust(WAR_DATA *war, CHAR_DATA *ch, CHAR_DATA *victim, int amount)
{
	WAR_DATA *iter;
	char buf[MSL];
	victim->clan->score -= amount;
	rec_event(war, EVENT_ADJUST_SCORE, ch->clan->clanname, victim->clan->clanname, amount);

	if (victim->clan->score < 1) {
		sprintf(buf, "%s has defeated %s!",
		        ch->clan->clanname, victim->clan->clanname);
		do_send_announce(ch, buf);
		ptc(ch, "You have defeated %s!\n\r", victim->clan->clanname);
		war_power_adjust(victim->clan, FALSE);

		for (iter = war_table_head->next; iter != war_table_tail; iter = iter->next)
			if (iter->ongoing && clan_in_war(victim->clan, iter, TRUE))
				defeat_clan(war, ch, victim);

		victim->clan->score = 0;

		if (!war->ongoing)
			stc("You have won the war!!\n\r", ch);
	}

	save_clan_table();
	save_war_table();
}

void war_kill(CHAR_DATA *ch, CHAR_DATA *victim)
{
	WAR_DATA *war;
	int points = 3;
	/* already verified that they're in clans, at war on opposite sides */
	war = get_same_war(ch->clan, victim->clan);
	rec_event(war, EVENT_KILL, ch->name, victim->name, 0);

	if (ch->pcdata->pkrank - victim->pcdata->pkrank > 3)       points = 1;
	else if (ch->pcdata->pkrank - victim->pcdata->pkrank > 1)       points = 2;
	else if (victim->pcdata->pkrank - ch->pcdata->pkrank > 3)       points = 4;
	else if (victim->pcdata->pkrank - ch->pcdata->pkrank > 1)       points = 5;

	war_score_adjust(war, ch, victim, points);
}

void rec_event(WAR_DATA *war, int type, char *astr, char *bstr, int number)
{
	EVENT_DATA *event, *n_event;
	n_event = new_event();
	n_event->type   = type;

	if (astr != NULL)
		n_event->astr = str_dup(astr);

	if (bstr != NULL)
		n_event->bstr = str_dup(bstr);

	n_event->number = number;
	n_event->time   = current_time;

	if (war->events == NULL) /* first event? */
		war->events = n_event;
	else {
		for (event = war->events; event != NULL; event = event->next)
			if (event->next == NULL) {
				event->next = n_event;
				break;
			}
	}

	save_war_events();
}

WAR_DATA *war_start(CLAN_DATA *chal, CLAN_DATA *def)
{
	WAR_DATA *war = new_war();

	if (chal->score <= 0)
		chal->score = calc_cp(chal, TRUE);

	if (def->score <= 0)
		def->score = calc_cp(def, TRUE);

	war->chal[0]->name              = str_dup(chal->name);
	war->chal[0]->clanname          = str_dup(chal->clanname);
	war->chal[0]->inwar             = TRUE;
	war->chal[0]->start_score       = chal->score;
	war->chal[0]->final_score       = 0;
	war->def[0]->name               = str_dup(def->name);
	war->def[0]->clanname           = str_dup(def->clanname);
	war->def[0]->inwar              = TRUE;
	war->def[0]->start_score        = def->score;
	war->def[0]->final_score        = 0;
	war->ongoing                    = TRUE;
	rec_event(war, EVENT_WAR_START, NULL, NULL, 0);
	append_war(war);
	return war;
}

void war_stop(WAR_DATA *war)
{
	int i;
	CLAN_DATA *clan;
	war->ongoing = FALSE;

	for (i = 0; i < 4; i++) {
		if (war->chal[i]->name[0] != '\0') {
			if ((clan = clan_lookup(war->chal[i]->name)) != NULL) {
				if (war->chal[i]->inwar)
					war->chal[i]->final_score = clan->score;

				if (!clan_at_war(clan))
					clan->score = 0;
			}
		}

		if (war->def[i]->name[0] != '\0') {
			if ((clan = clan_lookup(war->def[i]->name)) != NULL) {
				if (war->def[i]->inwar)
					war->def[i]->final_score = clan->score;

				if (!clan_at_war(clan))
					clan->score = 0;
			}
		}
	}
}

void war_win(WAR_DATA *war, CHAR_DATA *ch)
{
	rec_event(war, EVENT_WAR_STOP_WIN, NULL, NULL, clan_is_challenger(ch->clan, war));
	war_stop(war);
}

void war_join(CLAN_DATA *clan, WAR_DATA *war, bool challenger)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (challenger) {
			if (war->chal[i]->name[0] == '\0')
				break;
		}
		else {
			if (war->def[i]->name[0] == '\0')
				break;
		}
	}

	if (clan->score <= 0)
		clan->score = calc_cp(clan, TRUE);

	if (challenger) {
		war->chal[i]->name      = str_dup(clan->name);
		war->chal[i]->clanname  = str_dup(clan->clanname);
		war->chal[i]->inwar     = TRUE;
		war->chal[i]->start_score       = clan->score;
		war->chal[i]->final_score       = 0;
	}
	else {
		war->def[i]->name       = str_dup(clan->name);
		war->def[i]->clanname   = str_dup(clan->clanname);
		war->def[i]->inwar      = TRUE;
		war->def[i]->start_score        = clan->score;
		war->def[i]->final_score        = 0;
	}
}

void war_unjoin(CLAN_DATA *clan, WAR_DATA *war, bool remove)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (!str_cmp(war->chal[i]->name, clan->name)) {
			if (remove) {
				war->chal[i]->name      = &str_empty[0];
				war->chal[i]->clanname  = &str_empty[0];
				war->chal[i]->final_score       = 0;
			}
			else
				war->chal[i]->final_score       = clan->score;

			war->chal[i]->inwar = FALSE;
			break;
		}

		if (!str_cmp(war->def[i]->name, clan->name)) {
			if (remove) {
				war->def[i]->name       = &str_empty[0];
				war->def[i]->clanname   = &str_empty[0];
				war->def[i]->final_score        = 0;
			}
			else
				war->def[i]->final_score        = clan->score;

			war->def[i]->inwar = FALSE;
			break;
		}
	}

	if (remove)
		fix_war(war);

	if (!clan_at_war(clan))
		clan->score = 0;
}

void format_war_list(CHAR_DATA *ch, WAR_DATA *war, bool current)
{
	CLAN_DATA *clan;
	char chblock[MSL], defblock[MSL], buf[MSL], *vsblock;
	int i, x, chcount = 0, defcount = 0, c = 0, d = 0, chlead, deflead, lines;
	BUFFER *output;
	struct opp_list {
		char *name;
		bool inwar;
		int st_score;
		int score;
	};
	struct opp_list chal_list[4];
	struct opp_list def_list[4];

	/* count opponents on each side, get their full names and score */
	for (i = 0; i < 4; i++) {
		if (war->chal[i]->name[0] != '\0') {
			chal_list[chcount].name = war->chal[i]->clanname;
			chal_list[chcount].inwar = war->chal[i]->inwar;
			chal_list[chcount].st_score = war->chal[i]->start_score;

			if ((clan = clan_lookup(war->chal[i]->name)) != NULL
			    && war->chal[i]->inwar && current)
				chal_list[chcount].score = clan->score;
			else
				chal_list[chcount].score = war->chal[i]->final_score;

			chcount++;
		}

		if (war->def[i]->name[0] != '\0') {
			def_list[defcount].name = war->def[i]->clanname;
			def_list[defcount].inwar = war->def[i]->inwar;
			def_list[defcount].st_score = war->def[i]->start_score;

			if ((clan = clan_lookup(war->def[i]->name)) != NULL
			    && war->def[i]->inwar && current)
				def_list[defcount].score = clan->score;
			else
				def_list[defcount].score = war->def[i]->final_score;

			defcount++;
		}
	}

	if (chcount < 1 || defcount < 1) {
		bug("war status: war is one sided!", 0);
		return;
	}

	if (chcount > 4 || defcount > 4) {
		bug("war status: war has too many clans on one side!", 0);
		return;
	}

	sprintf(chblock, " ");
	sprintf(defblock, " ");
	vsblock = "";
	output = new_buf();
	chlead = (chcount - defcount);
	deflead = (defcount - chcount);
	lines = (chcount > defcount ? chcount : defcount);
	add_buf(output, "\n\r");

	for (x = 1; x < (lines + 1); x++) {
		if ((deflead > 1 && x == 1) || c >= chcount)
			sprintf(chblock, "                                      ");
		else if (c < chcount) {
			sprintf(chblock, "%s%30s %s(%s%2d{c/%s%2d%s)",
			        chal_list[c].inwar ? "" : "{c",
			        chal_list[c].inwar ? chal_list[c].name : smash_bracket(chal_list[c].name),
			        chal_list[c].inwar ? "{g" : "{c",
			        chal_list[c].inwar ? "{G" : "{P",
			        chal_list[c].score,
			        chal_list[c].inwar ? "{G" : "{P",
			        chal_list[c].st_score,
			        chal_list[c].inwar ? "{g" : "{c");
			c++;
		}

		if (chlead <= 1 && deflead <= 1 && chcount < 3 && defcount < 3) {
			if (x == 1)
				vsblock = "vs";
			else
				vsblock = "  ";
		}
		else {
			if (x == 2)
				vsblock = "vs";
			else
				vsblock = "  ";
		}

		if ((chlead > 1 && x == 1) || d >= defcount)
			sprintf(defblock, "                                      ");
		else if (d < defcount) {
			sprintf(defblock, "%s(%s%2d{c/%s%2d%s) %s%-30s",
			        def_list[d].inwar ? "{g" : "{c",
			        def_list[d].inwar ? "{G" : "{P",
			        def_list[d].score,
			        def_list[d].inwar ? "{G" : "{P",
			        def_list[d].st_score,
			        def_list[d].inwar ? "{g" : "{c",
			        def_list[d].inwar ? "{x" : "",
			        def_list[d].inwar ? def_list[d].name : smash_bracket(def_list[d].name));
			d++;
		}

		sprintf(buf, "%s{x    %s{x    %s{x\n\r", chblock, vsblock, defblock);
		add_buf(output, buf);
	}

	add_buf(output, "\n\r");
	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void format_war_events(CHAR_DATA *ch, WAR_DATA *war)
{
	char buf[MSL];
	BUFFER *output;
	EVENT_DATA *event;
	format_war_list(ch, war, war->ongoing);
	output = new_buf();

	for (event = war->events; event != NULL; event = event->next) {
		sprintf(buf, "{Punknown event type{x");

		switch (event->type) {
		case EVENT_WAR_START:
			sprintf(buf, "War started on %s\r", dizzy_ctime(&event->time));
			break;

		case EVENT_WAR_STOP_WIN:
			sprintf(buf, "The %s won the war on %s\r",
			        event->number ? "challengers" : "defenders",
			        dizzy_ctime(&event->time));
			break;

		case EVENT_WAR_STOP_IMM:
			sprintf(buf, "War was stopped by the Immortals on %s\r",
			        dizzy_ctime(&event->time));
			break;

		case EVENT_WAR_DECLARE:
			sprintf(buf, "%s has declared war on %s!\n\r",
			        event->astr, event->bstr);
			break;

		case EVENT_WAR_JOIN:
			sprintf(buf, "%s has joined the war on the %s side!\n\r",
			        event->astr, event->number ? "challenging" : "defending");
			break;

		case EVENT_KILL:
			sprintf(buf, "%s has been killed by %s!\n\r", event->bstr, event->astr);
			break;

		case EVENT_CLAN_DEFEAT:
			sprintf(buf, "%s has been defeated by %s!\n\r", event->astr, event->bstr);
			break;

		case EVENT_CLAN_WIPEOUT:
			sprintf(buf, "%s has been wiped out in war!\n\r", event->astr);
			break;

		case EVENT_CLAN_SURRENDER:
			sprintf(buf, "%s has surrendered.\n\r", event->astr);
			break;

		case EVENT_CLAN_INVADE:
			break;

		case EVENT_ADJUST_SCORE:
			sprintf(buf, "%s has lost %d points at the hands of %s!\n\r",
			        event->bstr, event->number, event->astr);
			break;

		case EVENT_ADJUST_POWER:
			sprintf(buf, "%s lost %d clanpower to %s.\n\r",
			        event->astr, event->number, event->bstr);
			break;

		case EVENT_ADJUST_CLANQP:
			sprintf(buf, "%s turned over %d questpoints to %s.\n\r",
			        event->astr, event->number, event->bstr);
			break;

		default:
			bug("format_war_events: Event type unknown", 0);
			break;
		}

		add_buf(output, buf);
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

/* all-encompassing war command */
void do_war(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[MSL];
	CLAN_DATA *clanA, *clanB;
	WAR_DATA *war = NULL;
	int count = 0, number = 0;
	bool challenger = FALSE;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	/*** HISTORY ***/
	if (!str_prefix1(arg1, "history")) {
		bool found = FALSE;
		war = war_table_head->next;

		while (war != war_table_tail) {
			count++;

			if (!war->ongoing) {
				ptc(ch, "{PWar %d:{x", count);
				format_war_list(ch, war, FALSE);
				found = TRUE;
			}

			war = war->next;
		}

		if (!found)
			stc("No wars have ended yet.\n\r", ch);

		return;
	}

	/*** LIST ***/
	if (!str_prefix1(arg1, "list")) {
		bool peace = TRUE;
		war = war_table_head->next;

		while (war != war_table_tail) {
			count++;

			if (war->ongoing) {
				ptc(ch, "{PWar %d:{x", count);
				format_war_list(ch, war, TRUE);
				peace = FALSE;
			}

			war = war->next;
		}

		if (peace)
			stc("Legacy is currently at peace.\n\r", ch);

		return;
	}

	/*** EVENTS ***/
	if (!str_prefix1(arg1, "events")) {
		if (arg2[0] == '\0') {
			stc("Syntax: war events <war number>\n\r", ch);
			return;
		}

		if (!is_number(arg2)) {
			stc("Use the number of the war.\n\r", ch);
			return;
		}

		number = atoi(arg2);

		if ((war = war_lookup(number)) == NULL) {
			stc("That is not a valid war.\n\r", ch);
			return;
		}

		format_war_events(ch, war);
		return;
	}

	/*** DECLARE ***/
	if (!str_cmp(arg1, "declare") && IS_IMMORTAL(ch)) {
		if (arg2[0] == '\0' || arg3[0] == '\0') {
			stc("Syntax: war declare <challenger> <defender>\n\r", ch);
			return;
		}

		if ((clanA = clan_lookup(arg2)) == NULL) {
			ptc(ch, "'%s' is not a clan.\n\r", arg2);
			return;
		}

		if ((clanB = clan_lookup(arg3)) == NULL) {
			ptc(ch, "'%s' is not a clan.\n\r", arg3);
			return;
		}

		if (clanA == clanB) {
			stc("They cannot declare war on themselves.\n\r", ch);
			return;
		}

		if (clanA->independent || clanB->independent) {
			stc("Independent clans cannot war.\n\r", ch);
			return;
		}

		if (calc_cp(clanA, FALSE) < 1) {
			stc("The challenger is not strong enough to wage war.\n\r", ch);
			return;
		}

		if (calc_cp(clanB, FALSE) < 1) {
			stc("The defender is already defeated.\n\r", ch);
			return;
		}

		if (clan_opponents(clanA, clanB)) {
			stc("They are already at war with eachother.\n\r", ch);
			return;
		}

		if (clan_at_war(clanA) || clan_at_war(clanB)) {
			bool war_found = FALSE;

			/* try to join the challenger to the defender's current war first */
			for (war = war_table_head->next; war != war_table_tail; war = war->next) {
				if (!war->ongoing)
					continue;

				if (!clan_in_war(clanB, war, TRUE))
					continue;

				if (clan_in_war(clanA, war, FALSE)) /* if they used to be */
					continue;

				war_join(clanA, war, !clan_is_challenger(clanB, war));
				war_found = TRUE;
				break;
			}

			/* now try to join the defender to the challenger's current war */
			if (!war_found)
				for (war = war_table_head->next; war != war_table_tail; war = war->next) {
					if (!war->ongoing)
						continue;

					if (!clan_in_war(clanB, war, TRUE))
						continue;

					if (clan_in_war(clanA, war, FALSE)) /* if they used to be */
						continue;

					war_join(clanA, war, !clan_is_challenger(clanB, war));
					war_found = TRUE;
					break;
				}

			/* wars are too full, start a new one */
			if (!war_found)
				war = war_start(clanA, clanB);
		}
		else
			war = war_start(clanA, clanB);

		rec_event(war, EVENT_WAR_DECLARE, clanA->clanname, clanB->clanname, 0);
		save_clan_table();
		save_war_table();
		ptc(ch, "%s is now at war with %s.\n\r",
		    clanA->clanname, clanB->clanname);
		sprintf(buf, "%s has declared war on %s!",
		        clanA->clanname, clanB->clanname);
		do_send_announce(ch, buf);
		return;
	}

	/*** JOIN ***/
	if (!str_cmp(arg1, "join") && IS_IMMORTAL(ch)) {
		if (arg2[0] == '\0' || arg3[0] == '\0' || argument[0] == '\0') {
			stc("Syntax: war join <clan> <war number> <challenger|defender>\n\r", ch);
			return;
		}

		if ((clanA = clan_lookup(arg2)) == NULL) {
			ptc(ch, "'%s' is not a clan.\n\r", arg2);
			return;
		}

		if (!is_number(arg3)) {
			stc("Use a number for the war.\n\r", ch);
			return;
		}

		number = atoi(arg3);

		if ((war = war_lookup(number)) == NULL) {
			stc("That is not a valid war.\n\r", ch);
			return;
		}

		if (!str_prefix1(argument, "challenger"))
			challenger = TRUE;
		else if (!str_prefix1(argument, "defender"))
			challenger = FALSE;
		else {
			stc("Use 'challenger' or 'defender' after the war number.\n\r", ch);
			return;
		}

		if (clanA->independent) {
			stc("Independent clans cannot war.\n\r", ch);
			return;
		}

		if (calc_cp(clanA, FALSE) < 1) {
			stc("The clan is not strong enough to wage war.\n\r", ch);
			return;
		}

		if (clan_in_war(clanA, war, FALSE)) {
			stc("They are already part of that war.\n\r", ch);
			return;
		}

		if (war_is_full(war, challenger)) {
			stc("That war is full.\n\r", ch);
			return;
		}

		war_join(clanA, war, challenger);
		rec_event(war, EVENT_WAR_JOIN, clanA->clanname, NULL, challenger);
		save_clan_table();
		save_war_table();
		ptc(ch, "%s is now a %s in War %d.\n\r",
		    clanA->clanname, challenger ? "challenger" : "defender", number);
		sprintf(buf, "%s has joined war %d!",
		        clanA->clanname, number);
		do_send_announce(ch, buf);
		return;
	}

	/*** UNJOIN ***/
	if (!str_cmp(arg1, "unjoin") && IS_IMMORTAL(ch)) {
		bool remove = FALSE;

		if (arg2[0] == '\0' || arg3[0] == '\0' || argument[0] == '\0') {
			stc("Syntax: war unjoin <clan> <war number> <remove|stop>\n\r", ch);
			return;
		}

		if ((clanA = clan_lookup(arg2)) == NULL) {
			ptc(ch, "'%s' is not a clan.\n\r", arg2);
			return;
		}

		if (!is_number(arg3)) {
			stc("Use the number of the war.\n\r", ch);
			return;
		}

		number = atoi(arg3);

		if ((war = war_lookup(number)) == NULL) {
			stc("That is not a valid war.\n\r", ch);
			return;
		}

		if (!str_prefix1(argument, "remove"))
			remove = TRUE;
		else if (!str_prefix1(argument, "stop"))
			remove = FALSE;
		else {
			stc("Use 'remove' or 'stop' after the war number.\n\r", ch);
			return;
		}

		if (!war->ongoing) {
			stc("That war has stopped.\n\r", ch);
			return;
		}

		war_unjoin(clanA, war, remove);
		save_clan_table();
		save_war_table();

		if (remove)
			stc("You remove them from the war.\n\r", ch);
		else
			stc("You stop their activity in the war.\n\r", ch);

		return;
	}

	/*** SURRENDER ***/
	/*      if (!str_prefix1(arg1, "surrender") && IS_IMMORTAL(ch))
	        {
	                if (arg2[0] == '\0')
	                {
	                        stc("Syntax: war surrender <clan>\n\r", ch);
	                        return;
	                }

	                if ((clanA = clan_lookup(arg2)) == NULL)
	                {
	                        ptc(ch, "'%s' is not a clan.\n\r", arg2);
	                        return;
	                }

	                if (!clan_at_war(clanA))
	                {
	                        stc("They are not at war.\n\r", ch);
	                        return;
	                }

	                war_surrender(clanA);

	                save_clan_table();
	                save_war_table();

	                stc("They pull out the white flag.\n\r", ch);
	                sprintf(buf, "%s has surrendered!", clanA->clanname);
	                do_send_announce(ch, buf);
	                return;
	        } */

	/*** STOP ***/
	if (!str_cmp(arg1, "stop") && IS_IMMORTAL(ch)) {
		if (arg2[0] == '\0') {
			stc("Syntax: war stop <war number>\n\r", ch);
			return;
		}

		if (!is_number(arg2)) {
			stc("Use the number of the war.\n\r", ch);
			return;
		}

		if ((number = atoi(arg2)) < 1) {
			stc("Number must be greater than 0.\n\r", ch);
			return;
		}

		if ((war = war_lookup(number)) == NULL) {
			stc("That is not a valid war.\n\r", ch);
			return;
		}

		if (!war->ongoing) {
			stc("The war has already stopped.\n\r", ch);
			return;
		}

		rec_event(war, EVENT_WAR_STOP_IMM, NULL, NULL, 0);
		war_stop(war);
		save_clan_table();
		save_war_table();
		stc("You stop the war.\n\r", ch);
		return;
	}

	/*** RELOAD ***/
	if (!str_cmp(arg1, "reload") && IS_IMP(ch)) {
		WAR_DATA *war_next;
		war = war_table_head;

		while (war != NULL) {
			war_next = war->next;
			free_war(war);
			war = war_next;
		}

		load_war_table();
		load_war_events();
		stc("War table reloaded.\n\r", ch);
		return;
	}

	/*      if (!str_cmp(arg1, "kill") && IS_IMMORTAL(ch))
	        {
	                CHAR_DATA *winner, *loser;

	                if (arg2[0] == '\0' || arg3[0] == '\0')
	                {
	                        stc("Syntax: war kill <char> <victim>\n\r", ch);
	                        return;
	                }

	                if ((winner = get_player_world(ch, arg2, VIS_PLR)) == NULL)
	                {
	                        stc("Winner not found.\n\r", ch);
	                        return;
	                }

	                if ((loser = get_player_world(ch, arg3, VIS_PLR)) == NULL)
	                {
	                        stc("Loser not found.\n\r", ch);
	                        return;
	                }

	                if (!char_opponents(winner, loser))
	                {
	                        stc("They are not at war.\n\r", ch);
	                        return;
	                }

	                war_kill(winner, loser);
	                return;
	        } */
	/* echo syntax */
	stc("Syntax:\n\r", ch);
	stc("  war list\n\r", ch);
	stc("  war history\n\r", ch);
	stc("  war events  <war number>\n\r", ch);

	if (IS_IMMORTAL(ch)) {
		stc("  war declare   <challenger> <defender>\n\r", ch);
		stc("  war join      <clan> <war number> <challenger|defender>\n\r", ch);
		stc("  war unjoin    <clan> <war number> <remove|stop>\n\r", ch);
//		stc("  war surrender <clan>\n\r", ch);
		stc("  war stop      <war number>\n\r", ch);
// Debugging
		/*              stc("  war kill    <char> <victim>\n\r", ch);
		                stc("  war reload            (reload the table from disk)\n\r", ch); */
	}
}


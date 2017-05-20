/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

/*************************************************
* IMM coding, testing, and debugging commands    *
* and related functions.  Most of these are in   *
* the coder command group.                       *
*************************************************/

#include "merc.h"
#include "recycle.h"
#include "memory.h"
#include "sql.h"
#include "affect.h"

DECLARE_DO_FUN(do_echo);

extern  time_t  reboot_time;
extern  int     top_exit;

/* externals for counting purposes */
extern  OBJ_DATA        *obj_free;
extern  CHAR_DATA       *char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern  PC_DATA         *pcdata_free;
extern  AFFECT_DATA     *affect_free;

void do_autoboot(CHAR_DATA *ch, const char *argument)
{
	char buf[MSL];
	int reboottime, hours = 0, minutes = 0;
	struct tm *tm;

	if (argument[0] == '\0') {
		if (reboot_time != 0)
			ptc(ch, "Legacy is scheduled to reboot at %s\n", (char *) ctime(&reboot_time));
		else
			stc("Legacy is not scheduled to be rebooted.\n", ch);

		stc("Syntax:\n"
		    "  autoreboot <numeric time>\n", ch);
		return;
	}

	if (!is_number(argument)) {
		stc("Time must be numeric, i.e. 1330 is 1:30 pm.\n", ch);
		return;
	}

	reboottime = atoi(argument);

	if (reboottime < 0 || reboottime > 2359) {
		stc("Time is out of range.\n", ch);
		return;
	}

	for (minutes = reboottime; minutes >= 59; minutes -= 100, hours += 1)
		;

	if (minutes < 0) {
		stc("The minutes must be under 60.\n", ch);
		return;
	}

	tm = localtime(&current_time);

	if ((hours < tm->tm_hour) || (hours == tm->tm_hour && minutes < tm->tm_min)) {
		stc("That time has already passed.\n"
		    "The mud will assume you want the reboot tomorrow.\n", ch);
		hours = hours + 24;
	}

	reboot_time = current_time;
	reboot_time = reboot_time + ((hours - tm->tm_hour) * 3600);
	reboot_time = reboot_time + ((minutes - tm->tm_min) * 60);
	ptc(ch, "Legacy is now scheduled to reboot at %s\n", (char *)ctime(&reboot_time));
	sprintf(buf, "%s called for auto-reboot at %s\n", ch->name, (char *)ctime(&reboot_time));
	fappend(SHUTDOWN_FILE, buf);
}

char *fgetf(char *s, int n, register FILE *iop)
{
	register int c;
	register char *cs;
	c = '\0';
	cs = s;

	while (--n > 0 && (c = getc(iop)) != EOF)
		if ((*cs++ = c) == '\0')
			break;

	*cs = '\0';
	return ((c == EOF && cs == s) ? NULL : s);
}

/* integrated shell */
void do_pipe(CHAR_DATA *ch, const char *argument)
{
	char buf[5000];
	FILE *fp;
	fp = popen(argument, "r");
	fgetf(buf, 5000, fp);
	page_to_char(buf, ch);
	pclose(fp);
	return;
}
/*
void do_mypipe(CHAR_DATA *ch, const char *argument)
{
	char divline[MSL], line[MSL];
	MYSQL_RES *result;
	MYSQL_FIELD *fields;
	MYSQL_ROW row[32000];
	BUFFER *output;
	int numfields = 0, numrows = 0, i, x;
	int lengths[100];

	for (i = 0; i < 100; i++)
		lengths[i] = 0;

	if (mysql_db == NULL) {
		bugf("do_mypipe:  mysql_db is NULL, reopening");
		db_open();
	}

	if (mysql_real_query(mysql_db, argument, strlen(argument))) {
		ptc(ch, "Error: %s\n", mysql_error(mysql_db));
		return;
	}

	result = mysql_store_result(mysql_db);
	numfields = mysql_field_count(mysql_db);

	if (!numfields) {
		ptc(ch, "%ld rows affected.\n", (long) mysql_affected_rows(mysql_db));
		mysql_free_result(result);
		return;
	}

	while ((row[numrows++] = mysql_fetch_row(result))) {
		unsigned long *reslengths = mysql_fetch_lengths(result);

		for (i = 0; i < numfields; i++)
			lengths[i] = UMAX(reslengths[i], lengths[i]);
	}

	output = new_buf();
	fields = mysql_fetch_fields(result);
	strcpy(divline, "{n ");

	for (i = 0; i < numfields; i++) {
		lengths[i] = UMAX(lengths[i], strlen(fields[i].name));

		for (x = 0; x <= lengths[i] + 2; x++)
			strcat(divline, " ");
	}

	strcat(divline, "{x\n");
	add_buf(output, divline);
	strcpy(line, "{n {x");

	for (i = 0; i < numfields; i++) {
		int wlen = lengths[i] - strlen(fields[i].name);
		strcat(line, " ");
		strcat(line, fields[i].name);

		while (wlen-- >= 0)
			strcat(line, " ");

		strcat(line, "{n {x");
	}

	strcat(line, "\n");
	add_buf(output, line);
	add_buf(output, divline);

	for (x = 0; x < numrows; x++) {
		strcpy(line, "{n {x");

		for (i = 0; i < numfields; i++) {
			int wlen = lengths[i] - color_strlen(row[x][i]);
			strcat(line, " ");

			if (row[x][i])
				strcat(line, row[x][i]);

			while (wlen-- >= 0)
				strcat(line, " ");

			strcat(line, "{n {x");
		}

		strcat(line, "\n");
		add_buf(output, line);
	}

	add_buf(output, divline);
	ptb(output, "  %ld rows in set.\n", (long) mysql_num_rows(result));
	page_to_char(buf_string(output), ch);
	free_buf(output);
	mysql_free_result(result);
}
*/
void do_reboo(CHAR_DATA *ch, const char *argument)
{
	stc("{NTo REBOOT, you must spell the entire word.{x\n", ch);
}

void do_reboot(CHAR_DATA *ch, const char *argument)
{
	char buf[MSL];
	extern bool merc_down;
	DESCRIPTOR_DATA *d, *d_next;

	if (argument[0] == '\0') {
		stc("You must provide a reason for a reboot.\n", ch);
		return;
	}

	set_color(ch, YELLOW, BOLD);
	sprintf(buf, "%s has called for a REBOOT.  Back in 60 seconds or less!\n", ch->name);
	do_echo(ch, buf);
	set_color(ch, WHITE, NOBOLD);
	do_allsave(ch, "");
	do_save(ch, "");
	merc_down = TRUE;

	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		close_socket(d);
	}
}

void do_shutdow(CHAR_DATA *ch, const char *argument)
{
	stc("{NTo SHUTDOWN, you must spell the entire word.{x\n", ch);
}

void do_shutdown(CHAR_DATA *ch, const char *argument)
{
	char buf[MSL], buf2[MSL];
	char *strtime;
	extern bool merc_down;
	DESCRIPTOR_DATA *d, *d_next;

	if (port == DIZZYPORT && !IS_IMP(ch)) {
		stc("You must be an implementor to shutdown Legacy.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("You must provide a reason for a shutdown.\n", ch);
		return;
	}

	sprintf(buf, "%s has SHUTDOWN the system.  Back after these messages!\n", ch->name);
	do_echo(ch, buf);

	if (port == DIZZYPORT) {
		strtime                    = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		sprintf(buf2, "%s :SHUTDOWN", strtime);
		fappend(SHUTDOWN_FILE, buf2);
		fappend(SHUTDOWN_FILE, argument);
	}

	do_allsave(ch, "");
	do_save(ch, "");
	merc_down = TRUE;

	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		close_socket(d);
	}
}

void do_slookup(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	int sn;

	if (argument[0] == '\0') {
		stc("Syntax:\n"
		    "slookup all\n"
		    "slookup <skill or spell name>\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!str_cmp(arg, "all")) {
		for (sn = 0; skill_table[sn].name != NULL; sn++)
			ptc(ch, "Sn: %3d  Slot: %4d  Skill/spell: '%s'\n",
			    sn,
			    skill_table[sn].slot,
			    skill_table[sn].name);

		return;
	}

	if ((sn = skill_lookup(arg)) < 0) {
		stc("No such skill or spell.\n", ch);
		return;
	}

	ptc(ch, "Sn: %3d  Slot: %4d  Skill/spell: '%s'\n",
	    sn,
	    skill_table[sn].slot,
	    skill_table[sn].name);
}

void do_advance(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;
	int level, iLevel;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		stc("Syntax:\n"
		    "  advance <player> <level>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL) {
		stc("They are not playing.\n", ch);
		return;
	}

	level = atoi(arg2);

	if (level < 1 || level > MAX_LEVEL) {
		ptc(ch, "Level must be 1 to %d.\n", MAX_LEVEL);
		return;
	}

	if (level == victim->level) {
		stc("It would accomplish nothing!\n", ch);
		return;
	}

	if (victim != ch) {
		if (IS_IMMORTAL(victim) && !IS_IMP(ch)) {
			stc("You must be an implementor to do that to a fellow immortal.\n", ch);
			return;
		}
	}

	if (level < victim->level) {
		stc("You revoke the levels of the player.\n", ch);
		stc("**** D'OH! D'OH! D'OH! ****\n", victim);

		for (iLevel = victim->level; iLevel > level; iLevel--) {
			stc("==>  ", victim);
			victim->level--;
			demote_level(victim);

			if (!IS_IMMORTAL(victim) && !IS_REMORT(victim)) {
				if (victim->level == (LEVEL_AVATAR - 1))
					REM_CGROUP(victim, GROUP_AVATAR);

				if (victim->level == (LEVEL_HERO - 1))
					REM_CGROUP(victim, GROUP_HERO);
			}
		}
	}
	else {
		stc("You bestow your Level power upon the character.\n", ch);
		stc("**** WoOhOo! WoOhOo! WoOhOo! ****\n", victim);

		for (iLevel = victim->level; iLevel < level; iLevel++) {
			stc("==>  ", victim);
			victim->level += 1;
			advance_level(victim);

			if (victim->level == LEVEL_AVATAR)
				SET_CGROUP(victim, GROUP_AVATAR);

			if (victim->level == LEVEL_HERO)
				SET_CGROUP(victim, GROUP_HERO);
		}
	}

	victim->exp = exp_per_level(victim, victim->pcdata->points) * UMAX(1, victim->level);
	save_char_obj(victim);
}

void do_wizlock(CHAR_DATA *ch, const char *argument)
{
	extern bool wizlock;
	wizlock = !wizlock;

	if (wizlock) {
		wiznet("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
		stc("Game wizlocked.\n", ch);
	}
	else {
		wiznet("$N removes the current wizlock.", ch, NULL, 0, 0, 0);
		stc("Game un-wizlocked.\n", ch);
	}
}

void do_relevel(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || !IS_SPECIAL(ch)) {
		do_huh(ch);
		return;
	}

	ch->pcdata->cgroup = GROUP_CLAN | GROUP_AVATAR | GROUP_HERO | GROUP_LEADER
	                     | GROUP_GEN | GROUP_QUEST | GROUP_BUILD | GROUP_CODE | GROUP_SECURE;
	ch->level       = MAX_LEVEL;
	ch->hit =  ATTR_BASE(ch, APPLY_HIT)     = 30000;
	ch->mana = ATTR_BASE(ch, APPLY_MANA)    = 30000;
	ch->stam = ATTR_BASE(ch, APPLY_STAM)    = 30000;
	stc("Done.\n", ch);
}

void do_addexit(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL];
	EXIT_DATA *exit;
	int dir;

	if (ch->in_room == NULL)
		return;

	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg1)) {
		stc("Syntax:\n"
		    "  addexit <to room vnum> <direction>\n", ch);
		return;
	}

	if (get_room_index(atoi(arg1)) == NULL) {
		stc("No such room with that vnum exists.\n", ch);
		return;
	}

	if (!str_prefix1(arg2, "north"))   dir = 0;
	else if (!str_prefix1(arg2, "east"))    dir = 1;
	else if (!str_prefix1(arg2, "south"))   dir = 2;
	else if (!str_prefix1(arg2, "west"))    dir = 3;
	else if (!str_prefix1(arg2, "up"))      dir = 4;
	else if (!str_prefix1(arg2, "down"))    dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	if (ch->in_room->exit[dir] != NULL) {
		stc("There is already an exit in that direction.\n", ch);
		return;
	}

	exit                            = (EXIT_DATA *)alloc_perm(sizeof(*exit));
	exit->description               = str_dup("");
	exit->keyword                   = str_dup("");
	exit->exit_info                 = 0;
	exit->key                       = -1;
	exit->u1.vnum                   = atoi(arg1);
	exit->u1.to_room                = get_room_index(exit->u1.vnum);
	ch->in_room->exit[dir]          = exit;
	ch->in_room->old_exit[dir]      = exit;
	top_exit++;
	stc("Exit added.\n", ch);
}

void do_remexit(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	int dir;

	if (ch->in_room == NULL)
		return;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax:\n"
		    "  remexit <direction>\n", ch);
		return;
	}

	if (!str_prefix1(arg, "north"))    dir = 0;
	else if (!str_prefix1(arg, "east"))     dir = 1;
	else if (!str_prefix1(arg, "south"))    dir = 2;
	else if (!str_prefix1(arg, "west"))     dir = 3;
	else if (!str_prefix1(arg, "up"))       dir = 4;
	else if (!str_prefix1(arg, "down"))     dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	if (ch->in_room->exit[dir] == NULL) {
		stc("There is no exit in that direction.\n", ch);
		return;
	}

	/* this is sloppy, but this command will be used once in a blue moon.
	   just abandon the exit :P */
	ch->in_room->exit[dir] = NULL;
	ch->in_room->old_exit[dir] = NULL;
	top_exit--;
	stc("Exit removed.\n", ch);
}

void do_sectchange(CHAR_DATA *ch, const char *argument)
{
	int sect;

	if (ch->in_room == NULL)
		return;

	if (argument[0] == '\0'
	    || !is_number(argument)
	    || (sect = atoi(argument)) < 0
	    || (sect > 10 && sect < 20)
	    || sect > 21) {
		stc("Syntax:\n"
		    "  sectchange <sector type number>\n\n"
		    "Current sector types are:\n"
		    "  0  inside\n"
		    "  1  city\n"
		    "  2  field\n"
		    "  3  forest\n"
		    "  4  hills\n"
		    "  5  mountain\n"
		    "  6  water_swim\n"
		    "  7  water_noswim\n"
		    "  8  (unused, don't pick)\n"
		    "  9  air\n"
		    " 10  desert\n"
		    " 20  arena\n"
		    " 21  clanarena\n", ch);
		return;
	}

	ch->in_room->sector_type = sect;
	stc("Sector type changed.\n", ch);
}

void do_memory(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "Affects %5d\n", top_affect);
	stc(buf, ch);
	sprintf(buf, "Areas   %5d\n", top_area);
	stc(buf, ch);
	sprintf(buf, "ExDes   %5d\n", top_ed);
	stc(buf, ch);
	sprintf(buf, "Exits   %5d\n", top_exit);
	stc(buf, ch);
	sprintf(buf, "Socials %5d\n", count_socials());
	stc(buf, ch);
	sprintf(buf, "Mobs    %5d\n", top_mob_index);
	stc(buf, ch);
	sprintf(buf, "(in use)%5d\n", mobile_count);
	stc(buf, ch);
	sprintf(buf, "Objs    %5d\n", top_obj_index);
	stc(buf, ch);
	sprintf(buf, "Resets  %5d\n", top_reset);
	stc(buf, ch);
	sprintf(buf, "Rooms   %5d\n", top_room);
	stc(buf, ch);
	sprintf(buf, "Shops   %5d\n", top_shop);
	stc(buf, ch);
	sprintf(buf, "Clans   %5d\n", count_clans());
	stc(buf, ch);
	sprintf(buf, "Characters in storage  %5d\n", count_stored_characters());
	stc(buf, ch);
	sprintf(buf, "Strings %5d strings of %7d bytes (max %d).\n",
	        nAllocString, sAllocString, MAX_STRING);
	stc(buf, ch);
	sprintf(buf, "Perms   %5d blocks  of %7d bytes.\n",
	        nAllocPerm, sAllocPerm);
	stc(buf, ch);
	return;
}

void do_dump(CHAR_DATA *ch, const char *argument)
{
	int count, count2, num_pcs, aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	PC_DATA *pc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	DESCRIPTOR_DATA *d;
	FILE *fp;
	int vnum, nMatch = 0;
	/* open file */
	fp = fopen("mem.dmp", "w");
	/* report use of data structures */
	num_pcs = 0;
	aff_count = 0;
	/* mobile prototypes */
	fprintf(fp, "MobProt %4d (%8ld bytes)\n",
	        top_mob_index, top_mob_index * (sizeof(*pMobIndex)));
	/* mobs */
	count = 0;  count2 = 0;

	for (fch = char_list; fch != NULL; fch = fch->next) {
		count++;

		if (fch->pcdata != NULL)
			num_pcs++;

		for (const AFFECT_DATA *af = affect_list_char(fch); af != NULL; af = af->next)
			aff_count++;
	}

	for (fch = char_free; fch != NULL; fch = fch->next)
		count2++;

	fprintf(fp, "Mobs    %4d (%8ld bytes), %2d free (%ld bytes)\n",
	        count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));
	/* pcdata */
	count = 0;

	for (pc = pcdata_free; pc != NULL; pc = pc->next)
		count++;

	fprintf(fp, "Pcdata  %4d (%8ld bytes), %2d free (%ld bytes)\n",
	        num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));
	/* descriptors */
	count = 0; count2 = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
		count++;

	for (d = descriptor_free; d != NULL; d = d->next)
		count2++;

	fprintf(fp, "Descs  %4d (%8ld bytes), %2d free (%ld bytes)\n",
	        count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
			for (const AFFECT_DATA *af = pObjIndex->affected; af != NULL; af = af->next)
				aff_count++;

			nMatch++;
		}

	fprintf(fp, "ObjProt %4d (%8ld bytes)\n",
	        top_obj_index, top_obj_index * (sizeof(*pObjIndex)));
	/* objects */
	count = 0;  count2 = 0;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		count++;

		for (const AFFECT_DATA *af = affect_list_obj(obj); af != NULL; af = af->next)
			aff_count++;
	}

	for (obj = obj_free; obj != NULL; obj = obj->next)
		count2++;

	fprintf(fp, "Objs    %4d (%8ld bytes), %2d free (%ld bytes)\n",
	        count, count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));
	/* affects */
	count = 0;

	for (const AFFECT_DATA *af = affect_free; af != NULL; af = af->next)
		count++;

	fprintf(fp, "Affects %4d (%8ld bytes), %2d free (%ld bytes)\n",
	        aff_count, aff_count * (sizeof(AFFECT_DATA)), count, count * (sizeof(AFFECT_DATA)));
	/* rooms */
	fprintf(fp, "Rooms   %4d (%8ld bytes)\n",
	        top_room, top_room * (sizeof(*room)));
	/* exits */
	fprintf(fp, "Exits   %4d (%8ld bytes)\n",
	        top_exit, top_exit * (sizeof(*exit)));
	fclose(fp);
	/* start printing out mobile data */
	fp = fopen("mob.dmp", "w");
	fprintf(fp, "\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;

	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			nMatch++;
			fprintf(fp, "#%-4d %3d active %3d killed     %s\n",
			        pMobIndex->vnum, pMobIndex->count,
			        pMobIndex->killed, pMobIndex->short_descr);
		}

	fclose(fp);
	/* start printing out object data */
	fp = fopen("obj.dmp", "w");
	fprintf(fp, "\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;

	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
			nMatch++;
			fprintf(fp, "#%-4d %3d active %3d reset      %s\n",
			        pObjIndex->vnum, pObjIndex->count,
			        pObjIndex->reset_num, pObjIndex->short_descr);
		}

	/* close file */
	fclose(fp);
}

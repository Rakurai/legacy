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

#include <vector>

#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "Reset.hh"
#include "Room.hh"
#include "Shop.hh"
#include "Social.hh"
#include "String.hh"
#include "comm.hh"
#include "World.hh"

extern  time_t  reboot_time;
extern  int     top_exit;

/* externals for counting purposes */
extern  Object        *obj_free;
extern  Character       *char_free;
extern  Descriptor *descriptor_free;
extern  Player         *pcdata_free;
extern  affect::Affect     *affect_free;

void do_autoboot(Character *ch, String argument)
{
	char buf[MSL];
	int reboottime, hours = 0, minutes = 0;
	struct tm *tm;

	if (argument.empty()) {
		if (reboot_time != 0)
			ptc(ch, "Legacy is scheduled to reboot at %s\n", (char *) ctime(&reboot_time));
		else
			stc("Legacy is not scheduled to be rebooted.\n", ch);

		stc("Syntax:\n"
		    "  autoreboot <numeric time>\n", ch);
		return;
	}

	if (!argument.is_number()) {
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

	tm = localtime(&Game::current_time);

	if ((hours < tm->tm_hour) || (hours == tm->tm_hour && minutes < tm->tm_min)) {
		stc("That time has already passed.\n"
		    "The mud will assume you want the reboot tomorrow.\n", ch);
		hours = hours + 24;
	}

	reboot_time = Game::current_time;
	reboot_time = reboot_time + ((hours - tm->tm_hour) * 3600);
	reboot_time = reboot_time + ((minutes - tm->tm_min) * 60);
	ptc(ch, "Legacy is now scheduled to reboot at %s\n", (char *)ctime(&reboot_time));
	Format::sprintf(buf, "%s called for auto-reboot at %s\n", ch->name, (char *)ctime(&reboot_time));
	fappend(SHUTDOWN_FILE, buf);
}

char *fgetf(char *s, int n, register FILE *iop)
{
	int c;
	char *cs;
	c = '\0';
	cs = s;

	while (--n > 0 && (c = getc(iop)) != EOF)
		if ((*cs++ = c) == '\0')
			break;

	*cs = '\0';
	return ((c == EOF && cs == s) ? nullptr : s);
}

/* integrated shell */
void do_pipe(Character *ch, String argument)
{
	char buf[5000];
	FILE *fp;
	fp = popen(argument.c_str(), "r");
	fgetf(buf, 5000, fp);
	page_to_char(buf, ch);
	pclose(fp);
	return;
}
/*
void do_mypipe(Character *ch, String argument)
{
	char divline[MSL], line[MSL];
	MYSQL_RES *result;
	MYSQL_FIELD *fields;
	MYSQL_ROW row[32000];
	String output;
	int numfields = 0, numrows = 0, i, x;
	int lengths[100];

	for (i = 0; i < 100; i++)
		lengths[i] = 0;

	if (mysql_db == nullptr) {
		Logging::bugf("do_mypipe:  mysql_db is nullptr, reopening");
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
			lengths[i] = std::max(reslengths[i], lengths[i]);
	}

	fields = mysql_fetch_fields(result);
	strcpy(divline, "{n ");

	for (i = 0; i < numfields; i++) {
		lengths[i] = std::max(lengths[i], strlen(fields[i].name));

		for (x = 0; x <= lengths[i] + 2; x++)
			divline += " ";
	}

	divline += "{x\n";
	output += divline;
	strcpy(line, "{n {x");

	for (i = 0; i < numfields; i++) {
		int wlen = lengths[i] - strlen(fields[i].name);
		line += " ";
		line += fields[i].name;

		while (wlen-- >= 0)
			line += " ";

		line += "{n {x";
	}

	line += "\n";
	output += line;
	output += divline;

	for (x = 0; x < numrows; x++) {
		strcpy(line, "{n {x");

		for (i = 0; i < numfields; i++) {
			int wlen = lengths[i] - row[x][i].uncolor().size();
			line += " ";

			if (row[x][i])
				line += row[x][i];

			while (wlen-- >= 0)
				line += " ";

			line += "{n {x";
		}

		line += "\n";
		output += line;
	}

	output += divline;
	output += Format::format("  %ld rows in set.\n", (long) mysql_num_rows(result));
	page_to_char(output, ch);
	mysql_free_result(result);
}
*/
void do_reboo(Character *ch, String argument)
{
	stc("{NTo REBOOT, you must spell the entire word.{x\n", ch);
}

void do_reboot(Character *ch, String argument)
{
	char buf[MSL];
	extern bool merc_down;
	Descriptor *d, *d_next;

	if (argument.empty()) {
		stc("You must provide a reason for a reboot.\n", ch);
		return;
	}

	set_color(ch, YELLOW, BOLD);
	Format::sprintf(buf, "%s has called for a REBOOT.  Back in 60 seconds or less!\n", ch->name);
	do_echo(ch, buf);
	set_color(ch, WHITE, NOBOLD);
	do_allsave(ch, "");
	do_save(ch, "");
	merc_down = true;

	for (d = descriptor_list; d != nullptr; d = d_next) {
		d_next = d->next;
		close_socket(d);
	}
}

void do_shutdow(Character *ch, String argument)
{
	stc("{NTo SHUTDOWN, you must spell the entire word.{x\n", ch);
}

void do_shutdown(Character *ch, String argument)
{
	char buf[MSL], buf2[MSL];
	char *strtime;
	extern bool merc_down;
	Descriptor *d, *d_next;

	if (Game::port == DIZZYPORT && !IS_IMP(ch)) {
		stc("You must be an implementor to shutdown Legacy.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("You must provide a reason for a shutdown.\n", ch);
		return;
	}

	Format::sprintf(buf, "%s has SHUTDOWN the system.  Back after these messages!\n", ch->name);
	do_echo(ch, buf);

	if (Game::port == DIZZYPORT) {
		strtime                    = ctime(&Game::current_time);
		strtime[strlen(strtime) - 1] = '\0';
		Format::sprintf(buf2, "%s :SHUTDOWN", strtime);
		fappend(SHUTDOWN_FILE, buf2);
		fappend(SHUTDOWN_FILE, argument);
	}

	do_allsave(ch, "");
	do_save(ch, "");
	merc_down = true;

	for (d = descriptor_list; d != nullptr; d = d_next) {
		d_next = d->next;
		close_socket(d);
	}
}

void do_slookup(Character *ch, String argument)
{
	skill::type sn;

	if (argument.empty()) {
		stc("Syntax:\n"
		    "slookup all\n"
		    "slookup <skill or spell name>\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg == "all") {
		for (const auto& pair : skill_table) {
			skill::type type = pair.first;
			const auto& entry = pair.second;

			if (type == skill::type::unknown)
				continue;

			ptc(ch, "Sn: %3d  Skill/spell: '%s'\n",
			    (int)type,
			    entry.name);
		}

		return;
	}

	if ((sn = skill::lookup(arg)) == skill::type::unknown) {
		stc("No such skill or spell.\n", ch);
		return;
	}

	ptc(ch, "Sn: %3d  Skill/spell: '%s'\n",
	    sn,
	    skill::lookup(sn).name);
}

void do_advance(Character *ch, String argument)
{
	Character *victim;
	int level, iLevel;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty() || !arg2.is_number()) {
		stc("Syntax:\n"
		    "  advance <player> <level>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
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
					victim->remove_cgroup(GROUP_AVATAR);

				if (victim->level == (LEVEL_HERO - 1))
					victim->remove_cgroup(GROUP_HERO);
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
				victim->add_cgroup(GROUP_AVATAR);

			if (victim->level == LEVEL_HERO)
				victim->add_cgroup(GROUP_HERO);
		}
	}

	victim->exp = exp_per_level(victim, victim->pcdata->points) * std::max(1, victim->level);
	save_char_obj(victim);
}

void do_wizlock(Character *ch, String argument)
{
	Game::wizlock = !Game::wizlock;

	if (Game::wizlock) {
		wiznet("$N has wizlocked the game.", ch, nullptr, 0, 0, 0);
		stc("Game wizlocked.\n", ch);
	}
	else {
		wiznet("$N removes the current wizlock.", ch, nullptr, 0, 0, 0);
		stc("Game un-wizlocked.\n", ch);
	}
}

void do_relevel(Character *ch, String argument)
{
	if (ch->is_npc() || !IS_SPECIAL(ch)) {
		do_huh(ch);
		return;
	}

	ch->pcdata->cgroup_flags = GROUP_CLAN | GROUP_AVATAR | GROUP_HERO | GROUP_LEADER
	                     | GROUP_GEN | GROUP_QUEST | GROUP_BUILD | GROUP_CODE | GROUP_SECURE;
	ch->level       = MAX_LEVEL;
	ch->hit =  ATTR_BASE(ch, APPLY_HIT)     = 30000;
	ch->mana = ATTR_BASE(ch, APPLY_MANA)    = 30000;
	ch->stam = ATTR_BASE(ch, APPLY_STAM)    = 30000;
	stc("Done.\n", ch);
}

void do_addexit(Character *ch, String argument)
{
	stc("Sorry, adding and removing exits is currently disabled.\n", ch);
	/*
	Exit *exit;
	int dir;

	if (ch->in_room == nullptr)
		return;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty() || !arg1.is_number()) {
		stc("Syntax:\n"
		    "  addexit <to room vnum> <direction>\n", ch);
		return;
	}

	if (Game::world().get_room(atoi(arg1)) == nullptr) {
		stc("No such room with that vnum exists.\n", ch);
		return;
	}

	if (arg2.is_prefix_of("north"))   dir = 0;
	else if (arg2.is_prefix_of("east"))    dir = 1;
	else if (arg2.is_prefix_of("south"))   dir = 2;
	else if (arg2.is_prefix_of("west"))    dir = 3;
	else if (arg2.is_prefix_of("up"))      dir = 4;
	else if (arg2.is_prefix_of("down"))    dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	if (ch->in_room->exit[dir] != nullptr) {
		stc("There is already an exit in that direction.\n", ch);
		return;
	}

	exit = new Exit;
	exit->key                       = -1;
	exit->u1.vnum                   = atoi(arg1);
	exit->to_room                = Game::world().get_room(exit->u1.vnum);
	ch->in_room->exit[dir]          = exit;
	top_exit++;
	stc("Exit added.\n", ch);
	*/
}

void do_remexit(Character *ch, String argument)
{
	stc("Sorry, adding and removing exits is currently disabled.\n", ch);
	/*
	int dir;

	if (ch->in_room == nullptr)
		return;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n"
		    "  remexit <direction>\n", ch);
		return;
	}

	if (arg.is_prefix_of("north"))    dir = 0;
	else if (arg.is_prefix_of("east"))     dir = 1;
	else if (arg.is_prefix_of("south"))    dir = 2;
	else if (arg.is_prefix_of("west"))     dir = 3;
	else if (arg.is_prefix_of("up"))       dir = 4;
	else if (arg.is_prefix_of("down"))     dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	if (ch->in_room->exit[dir] == nullptr) {
		stc("There is no exit in that direction.\n", ch);
		return;
	}

	// this is sloppy, but this command will be used once in a blue moon.
	// just abandon the exit :P 
	ch->in_room->exit[dir] = nullptr;
	top_exit--;
	stc("Exit removed.\n", ch);
*/
}

void do_memory(Character *ch, String argument)
{
	ptc(ch, "Affects %5d allocated, %5d free, %5d B each\n", affect::Affect::pool_allocated(), affect::Affect::pool_free(), sizeof(affect::Affect));
	ptc(ch, "Areas   %5d,                       %5d B each\n", Game::world().areas.size(), sizeof(Area));
	ptc(ch, "ExDes   %5d allocated, %5d free, %5d B each\n", ExtraDescr::pool_allocated(), ExtraDescr::pool_free(), sizeof(ExtraDescr));
	ptc(ch, "Exits   %5d,                       %5d B each\n", top_exit, sizeof(Exit));
	ptc(ch, "Socials %5d,                       %5d B each\n", count_socials(), sizeof(Social));
	ptc(ch, "Chars   %5d allocated, %5d free, %5d B each\n", Character::pool_allocated(), Character::pool_free(), sizeof(Character));
	ptc(ch, "Descrs  %5d allocated, %5d free, %5d B each\n", Descriptor::pool_allocated(), Descriptor::pool_free(), sizeof(Descriptor));
	ptc(ch, "Objs    %5d allocated, %5d free, %5d B each\n", Object::pool_allocated(), Object::pool_free(), sizeof(Object));
	ptc(ch, "Resets  %5d,                       %5d B each\n", top_reset, sizeof(Reset));
	ptc(ch, "Rooms   %5d,                       %5d B each\n", top_room, sizeof(Room));
	ptc(ch, "Shops   %5d,                       %5d B each\n", top_shop, sizeof(Shop));
	ptc(ch, "Clans   %5d,                       %5d B each\n", count_clans(), sizeof(Clan));
	ptc(ch, "Characters in storage  %5d\n", count_stored_characters());
	return;
}

void do_dump(Character *ch, String argument)
{
	int count, num_pcs, aff_count;
	MobilePrototype *pMobIndex;
	Object *obj;
	ObjectPrototype *pObjIndex;
	Room *room;
	Exit *exit;
	Descriptor *d;
	FILE *fp;
	int vnum, nMatch = 0;
	/* open file */
	fp = fopen("mem.dmp", "w");
	/* report use of data structures */
	num_pcs = 0;
	aff_count = 0;
	/* mobile prototypes */
	Format::fprintf(fp, "MobProt %4d (%8ld bytes)\n",
	        top_mob_index, top_mob_index * (sizeof(*pMobIndex)));
	/* mobs */
	count = 0;

	for (auto fch : Game::world().char_list) {
		count++;

		if (fch->pcdata != nullptr)
			num_pcs++;

		for (const affect::Affect *af = affect::list_char(fch); af != nullptr; af = af->next)
			aff_count++;
	}

	Format::fprintf(fp, "Mobs    %4d (%8ld bytes)\n",
	        count, count * (sizeof(Character)));
	/* pcdata */

	Format::fprintf(fp, "Pcdata  %4d (%8ld bytes)\n",
	        num_pcs, num_pcs * (sizeof(Player)));
	/* descriptors */
	count = 0;

	for (d = descriptor_list; d != nullptr; d = d->next)
		count++;

	Format::fprintf(fp, "Descs  %4d (%8ld bytes)\n",
	        count, count * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = Game::world().get_obj_prototype(vnum)) != nullptr) {
			for (const affect::Affect *af = pObjIndex->affected; af != nullptr; af = af->next)
				aff_count++;

			nMatch++;
		}

	Format::fprintf(fp, "ObjProt %4d (%8ld bytes)\n",
	        top_obj_index, top_obj_index * (sizeof(*pObjIndex)));
	/* objects */
	count = 0;

	for (obj = Game::world().object_list; obj != nullptr; obj = obj->next) {
		count++;

		for (const affect::Affect *af = affect::list_obj(obj); af != nullptr; af = af->next)
			aff_count++;
	}

	Format::fprintf(fp, "Objs    %4d (%8ld bytes)\n",
	        count, count * (sizeof(*obj)));
	/* affects */
	Format::fprintf(fp, "Affects %4d (%8ld bytes)\n",
	        aff_count, aff_count * (sizeof(affect::Affect)));
	/* rooms */
	Format::fprintf(fp, "Rooms   %4d (%8ld bytes)\n",
	        top_room, top_room * (sizeof(*room)));
	/* exits */
	Format::fprintf(fp, "Exits   %4d (%8ld bytes)\n",
	        top_exit, top_exit * (sizeof(*exit)));
	fclose(fp);
	/* start printing out mobile data */
	fp = fopen("mob.dmp", "w");
	Format::fprintf(fp, "\nMobile Analysis\n");
	Format::fprintf(fp,  "---------------\n");
	nMatch = 0;

	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = Game::world().get_mob_prototype(vnum)) != nullptr) {
			nMatch++;
			Format::fprintf(fp, "#%-4d %3d active %3d killed     %s\n",
			        pMobIndex->vnum, pMobIndex->count,
			        pMobIndex->killed, pMobIndex->short_descr);
		}

	fclose(fp);
	/* start printing out object data */
	fp = fopen("obj.dmp", "w");
	Format::fprintf(fp, "\nObject Analysis\n");
	Format::fprintf(fp,  "---------------\n");
	nMatch = 0;

	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = Game::world().get_obj_prototype(vnum)) != nullptr) {
			nMatch++;
			Format::fprintf(fp, "#%-4d %3d active %3d reset      %s\n",
			        pObjIndex->vnum, pObjIndex->count,
			        pObjIndex->reset_num, pObjIndex->short_descr);
		}

	/* close file */
	fclose(fp);
}

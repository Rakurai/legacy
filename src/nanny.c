#include "merc.h"
#include "vt100.h"
#include "sql.h"
#include "recycle.h"

//For the hack fix

DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN(do_unread);

bool    check_parse_name        args((char *name));
bool    check_player_exist      args((DESCRIPTOR_DATA *d, char *name));
bool    check_reconnect         args((DESCRIPTOR_DATA *d, char *name, bool fConn));
bool    check_playing           args((DESCRIPTOR_DATA *d, char *name));
int     roll_stat               args((CHAR_DATA *ch, int stat));

extern void     roll_raffects   args((CHAR_DATA *ch, CHAR_DATA *victim));
extern void     goto_line    args((CHAR_DATA *ch, int row, int column));
extern void     set_window   args((CHAR_DATA *ch, int top, int bottom));

bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */

/*
 * Socket and TCP/IP stuff.
 */
#if     defined(unix)
#include "telnet.h"
#if defined (SAND)
#endif
#if !defined( STDOUT_FILENO )
#define STDOUT_FILENO 1
#endif
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
/* Overflow messages are due to fact this is not declared as unsigned...however, declaring
as unsigned causes errors in write_to_buf.  Therefore, ignore the warning - Lotus */
#endif

/**
 * check_ban
 * Run from nanny() to check if a site is banned.
 */
bool check_ban(char *site, int type)
{
	bool ban = FALSE;

	if (db_queryf("check_ban", "SELECT flags, site FROM bans WHERE ((flags>>2)<<2)=%d", type) != SQL_OK)
		return FALSE;

	while (!ban && db_next_row() == SQL_OK) {
		int flags = db_get_column_int(0);
		const char *str = db_get_column_str(1);
		bool prefix = IS_SET(flags, BAN_PREFIX);
		bool suffix = IS_SET(flags, BAN_SUFFIX);

		if ((prefix  &&  suffix && !str_infix(str, site))
		    || (prefix  && !suffix && !str_suffix(str, site))
		    || (!prefix &&  suffix && !str_prefix(str, site))
		    || (!prefix && !suffix && !str_cmp(str, site)))
			ban = TRUE;
	}

	return ban;
}

bool check_deny(char *name)
{
	if (db_countf("check_deny", "SELECT COUNT(*) FROM denies WHERE name='%s'", name) <= 0)
		return FALSE;

	return TRUE;
}

void update_site(CHAR_DATA *ch)
{
	char shortsite[MSL];
	extern char *site_to_ssite(char *site);
	strcpy(shortsite, site_to_ssite(ch->desc->host));

	if (db_countf("update_site",
	              "SELECT COUNT(*) FROM sites WHERE name='%s' AND ssite='%s'", ch->name, shortsite) > 0)
		db_commandf("update_site", "UPDATE sites SET lastlog=DATE(), site='%s' WHERE name='%s' AND ssite='%s'",
		            db_esc(ch->desc->host), db_esc(ch->name), db_esc(shortsite));
	else
		db_commandf("update_site", "INSERT INTO sites VALUES('%s','%s','%s',DATE())",
		            db_esc(ch->name), db_esc(ch->desc->host), db_esc(shortsite));
}

unsigned long update_records()
{
	DESCRIPTOR_DATA *d;
	int count = 0;

	if (port != DIZZYPORT)
		return 0;

	db_command("update_records", "UPDATE records SET logins=logins+1");

	for (d = descriptor_list; d != NULL; d = d->next)
		if (IS_PLAYING(d))
			count++;

	record_players_since_boot = UMAX(count, record_players_since_boot);

	if (record_players_since_boot > record_players) {
		record_players = record_players_since_boot;
		db_commandf("update_records", "UPDATE records SET players=%d", record_players_since_boot);
	}

	return ++record_logins;
}

void update_pc_index(CHAR_DATA *ch, bool remove)
{
	db_commandf("update_pc_index", "DELETE FROM pc_index WHERE name='%s'", db_esc(ch->name));

	if (!remove)
		db_commandf("update_pc_index",
		            "INSERT INTO pc_index VALUES('%s','%s','%s','%s',%ld,%d,%d,'%s','%s')",
		            db_esc(ch->name),
		            db_esc(ch->pcdata->title),
		            db_esc(ch->pcdata->deity),
		            db_esc(smash_bracket(ch->pcdata->deity)),
		            ch->pcdata->cgroup,
		            ch->level,
		            ch->pcdata->remort_count,
		            ch->clan ? db_esc(ch->clan->name) : "",
		            ch->clan && ch->pcdata->rank ? db_esc(ch->pcdata->rank) : "");
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA *d, const char *argument)
{
	DESCRIPTOR_DATA *d_old, *d_next, *sd;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch, *victim;
	const char *pwdnew, *p;
	int iClass, race, i, weapon, deity;
	bool fOld, logon_lurk;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	switch (d->connected) {
	default:
		bug("Nanny: bad d->connected %d.", d->connected);
		close_socket(d);
		return;

	case CON_GET_NAME:
		if (argument[0] == '\0') {
			close_socket(d);
			return;
		}

		logon_lurk = FALSE;

		if (argument[0] == '-') { /* Lurk mode -- Elrac */
			logon_lurk = TRUE;
			argument++;
		}

		char name[MIL];
		strcpy(name, argument);
		name[0] = UPPER(name[0]);

		/* Check valid name - Lotus */
		if (!check_parse_name(name)) {
			write_to_buffer(d, "Sorry, that name cannot be used.\n"
			                "Please choose another name!\n"
			                "\n"
			                "Name: ", 0);
			free_char(d->character);
			d->character = NULL;
			return;
		}

		if (check_player_exist(d, name))
			return;

		/* Below this point we have a character, we can use stc */
		/********************************************************/
		fOld = load_char_obj(d, name);
		ch   = d->character;

		/********************************************************/

		/* check for attempt to newly create with a mob name -- Elrac */
		if (!fOld && mob_exists(name)) {
			write_to_buffer(d, "Sorry, we already have a mobile by that name.\n"
			                "Please choose another name!\n"
			                "\n"
			                "Name: ", 0);
			free_char(ch);
			d->character = NULL;
			return;
		}

		if (check_deny(ch->name)) {
			sprintf(log_buf, "Denying access to %s@%s.", ch->name, d->host);
			log_string(log_buf);
			write_to_buffer(d, "You are denied access to Legacy.\n", 0);
			close_socket(d);
			return;
		}

		if (check_ban(d->host, BAN_ALL) && !IS_SET(ch->act, PLR_PERMIT)) {
			sprintf(log_buf, "Disconnecting because BANned: %s", d->host);
			log_string(log_buf);
			wiznet(log_buf, NULL, NULL, WIZ_LOGINS, 0, 0);
			write_to_buffer(d, "Your site has been banned from this mud.\n"
			                "If you feel that your site has been banned in error, or would\n"
			                "like to request special permission to play, please contact us at:\n"
			                "   legacy@kyndig.com\n", 0);
			close_socket(d);
			return;
		}

		if (check_reconnect(d, name, FALSE))
			fOld = TRUE;
		else if (wizlock && !IS_IMMORTAL(ch)) {
			write_to_buffer(d, "Access has been limited to imms only at this time.\n", 0);
			close_socket(d);
			return;
		}

		if (fOld) { /* old player */
			if (logon_lurk && IS_IMMORTAL(ch))
				ch->lurk_level = 92;

			write_to_buffer(d, "What is your password? ", 0);
			write_to_buffer(d, echo_off_str, 0);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}

		if (newlock) {
			write_to_buffer(d, "Due to technical difficulties, we are not accepting new players\n"
			                "at this time.  Please try again in a few hours.\n", 0);
			close_socket(d);
			return;
		}

		if (check_ban(d->host, BAN_NEWBIES)) {
			sprintf(log_buf, "Disconnecting because NewbieBANned: %s", d->host);
			log_string(log_buf);
			wiznet(log_buf, NULL, NULL, WIZ_LOGINS, 0, 0);
			write_to_buffer(d, "New players are not allowed from your site.\n"
			                "If you feel that your site has been banned in error, or would\n"
			                "like to request special permission to play, please contact us at:\n"
			                "   legacyimms@kyndig.com\n", 0);
			close_socket(d);
			return;
		}

		sprintf(buf, "You wish for history to remember you as %s (Y/N)? ", name);
		write_to_buffer(d, buf, 0);
		d->connected = CON_CONFIRM_NEW_NAME;
		return;

	case CON_GET_OLD_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n", 2);
#endif

		if (strcmp(argument, ch->pcdata->pwd)) {
			stc("{bIncorrect password!{x\n", ch);
			close_socket(d);
			return;
		}

		write_to_buffer(d, echo_on_str, 0);

		if (check_playing(d, ch->name))
			return;

		if (check_reconnect(d, ch->name, TRUE))
			return;

		REMOVE_BIT(ch->pcdata->plr, PLR_LINK_DEAD);
		REMOVE_BIT(ch->pcdata->plr, PLR_SQUESTOR);
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		sprintf(log_buf, "%s@%s has connected.", ch->name, d->host);
		log_string(log_buf);
		wiznet(log_buf, NULL, NULL, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));
		sprintf(log_buf, "Last Site: %s",
		        (ch->pcdata->last_lsite != NULL &&
		         ch->pcdata->last_lsite[0] != '\0') ? ch->pcdata->last_lsite : "Not Available");
		wiznet(log_buf, NULL, NULL, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));
		update_site(ch);

		if (IS_IMMORTAL(ch)) {
			set_color(ch, RED, BOLD);
			help(ch, "imotd");
			set_color(ch, WHITE, NOBOLD);
			d->connected = CON_READ_MOTD;
		}
		else {
			set_color(ch, CYAN, NOBOLD);
			help(ch, "automotd");
			set_color(ch, WHITE, NOBOLD);
			d->connected = CON_READ_MOTD;
		}

		break;

	case CON_BREAK_CONNECT:
		switch (*argument) {
		case 'y':
		case 'Y':
			for (d_old = descriptor_list; d_old != NULL; d_old = d_next) {
				d_next = d_old->next;

				if (d_old == d || d_old->character == NULL)
					continue;

				if (str_cmp(ch->name, d_old->original ?
				            d_old->original->name : d_old->character->name))
					continue;

				close_socket(d_old);
			}

			if (check_reconnect(d, ch->name, TRUE))
				return;

			write_to_buffer(d, "Reconnect attempt failed.\nName: ", 0);

			if (d->character != NULL) {
				free_char(d->character);
				d->character = NULL;
			}

			d->connected = CON_GET_NAME;
			break;

		case 'n':
		case 'N':
			write_to_buffer(d, "Name: ", 0);

			if (d->character != NULL) {
				free_char(d->character);
				d->character = NULL;
			}

			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Y or N? ", 0);
			break;
		}

		break;

	case CON_CONFIRM_NEW_NAME:

		/* We're past all the reconnect stuff, we can be sure that the character
		   is not playing and is new.  Start using stc and ptc for the colors */
		switch (*argument) {
		case 'y':
		case 'Y':
			stc("\nYou find yourself standing in the market square of Midgaard, surrounded by\n"
			    "the exotic scents and bright colors of the open bazaar.  A tiny pixie flits\n"
			    "through the air and hovers in front of you, looking you over approvingly.\n\n", ch);
			ptc(ch, "{Y'{WWelcome to Legacy, %s!{Y'{x she says, smiling happily.  {Y'{WI hope you have\n"
			    " some free time, the world is a big place and there is so much to do!{Y'{x\n\n", ch->name);
			stc("{Y'{WFirst things first, though!  Tell me, how familiar are you with Legacy?{Y'{x\n\n", ch);
			stc("{Y1){x ... what is this place?                  ({YNew to MUDs{x)\n"
			    "{Y2){x I've never been to this realm.           ({YNew to Legacy{x)\n"
			    "{Y3){x Let's go, I know everything!             ({YExperienced player{x)\n\n"
			    "{CPlease answer {Y1{C, {Y2{C, or {Y3{C.{x ", ch);
			d->connected = CON_GET_MUD_EXP;
			break;

		case 'n':
		case 'N':
			stc("Ok, what do you want to be called, then? ", ch);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			stc("Please type Yes or No? ", ch);
			break;
		}

		break;

	case CON_GET_MUD_EXP:
		switch (*argument) {
		case '1':
			stc("\nThe pixie beams a smile at you, and you shuffle your feet nervously.\n\n"
			    "{Y'{WWell then,{Y'{x she says, {Y'{Wyou've been missing out!  Legacy is a MUD, a place\n"
			    " where you can walk, talk, and interact with people all over the world in a\n"
			    " fantasy setting.  You've taken the first step, as we all did.  I will warn\n"
			    " you, though, this place is addicting, you may not want to leave.{Y'{x  She winks\n"
			    " at you, and you find yourself following her toward a massive white marble\n"
			    " temple just north of the marketplace.\n\n", ch);
			stc("{Y'{WBefore we continue, we need to tell your password to the city guards.\n"
			    " Every time you come back to visit us, you will have to speak your password\n"
			    " before they will let you in.  Think of a good one, so nobody can pretend\n"
			    " to be you!  Just whisper it in my ear, and we'll make sure they record it.{Y'{x\n\n", ch);
			ch->pcdata->mud_exp = MEXP_TOTAL_NEWBIE;
			break;

		case '2':
			stc("\nThe pixie beams a smile at you.\n\n"
			    "{Y'{WAllow me, then, to show you around!  We'll get you acquainted in no time.{Y'{x\n\n"
			    "You start to follow her toward a massive white marble temple just north of\n"
			    "the marketplace.\n\n"
			    "{Y'{WBefore we continue, we need to tell your password to the city guards, so\n"
			    " they will let you in when you come back to visit us.  Just whisper it in my\n"
			    " ear, and we'll make sure they record it.{Y'{x\n\n", ch);
			ch->pcdata->mud_exp = MEXP_LEGACY_NEWBIE;
			break;

		case '3':
			stc("\nThe pixie grins at you.\n\n"
			    "{Y'{WWell, then, there is no time to lose!  I'm sure you know how strict the\n"
			    " city guards can be, let's give them your password.  Whisper it in my ear,\n"
			    " we'll make sure they record it.{Y'{x\n\n", ch);
			ch->pcdata->mud_exp = MEXP_LEGACY_OLDBIE;
			break;

		default:
			stc("{CPlease answer {Y1{C, {Y2{C, or {Y3{C.{x ", ch);
			return;
		}

		d->connected = CON_GET_NEW_PASSWORD;
		ptc(ch, "{CWhat password do you tell her?{x %s", echo_off_str);
		break;

	case CON_GET_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n", 2);
#endif

		if (strlen(argument) < 5) {
			stc("You whisper in her ear, and she giggles.\n\n"
			    "{Y'{WThat's too short, somebody could guess it easily.  Make up one that's at"
			    " least five letters long!{Y'{x she says, and flies up close to hear you whisper.\n\n", ch);
			stc("{CWhat password do you tell her?{x ", ch);
			return;
		}

		pwdnew = argument;

		for (p = pwdnew; *p != '\0'; p++)
			if (*p == '~') {
				stc("She frowns as you whisper in her ear.\n\n"
				    "{Y'{WI'm not sure how you managed to pronounce that, but I'm sure the guards\n"
				    " would not recognize it.  Try something with only letters and numbers, maybe?{Y'{x\n"
				    "She leans close for you to whisper another password.\n\n", ch);
				stc("{CWhat password do you tell her?{x ", ch);
				return;
			}

		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd = str_dup(pwdnew);
		stc("The pixie smiles as you whisper in her ear.\n\n"
		    "{Y'{WGood, that will do nicely.  Just to make sure I have it, say it one more time.{Y'{x\n\n", ch);
		stc("{CPlease repeat the password.{x ", ch);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n", 2);
#endif

		if (strcmp(argument, ch->pcdata->pwd)) {
			stc("The pixie frowns as you whisper again.\n\n"
			    "{Y'{WThat does not sound like the first password you told me... let's try again.\n"
			    " What password do you want to use?{Y'{x she says as she flies closer to hear.\n\n", ch);
			stc("{CWhat password do you tell her?{x ", ch);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}

		ptc(ch, "%sShe nods her approval.\n\n", echo_on_str);
		ptc(ch, "{Y'{WYes, that's what I thought you said.{Y'{x  As your walk draws close to the grand\n"
		    " marble steps leading to the temple, she flies over to a nearby cityguard and\n"
		    " taps him on the ear.  {Y'{WGood morning!{Y'{x she says.  {Y'{W%s here is a friend of\n"
		    " mine, and needs to be added to the list of citizens.  The password is...{Y'{x She\n"
		    " flies close to the cityguard's ear and whispers, then flits back to your side.\n\n", ch->name);
		stc("She leads you to the first of the temple steps, and lands gracefully on the\n"
		    "carved handrail.  She gestures for you to take a seat.\n\n"
		    "{Y'{WThe temple ahead is a place of worship, and also a place of learning.\n"
		    " Before we go there, though, tell me about yourself.{Y'{x  She smiles.  {Y'{WI can\n"
		    "see that you are a...{Y'{x\n\n", ch);
		stc("Here you may choose your race.  If you are unsure of what you would like\n"
		    "to be, just type {Yhelp{x for information on the races.\n\n", ch);
		stc("The following races are available:\n", ch);

		for (race = 1; race_table[race].name != NULL && race_table[race].pc_race; race++)
			if (!pc_race_table[race].remort_level)
				ptc(ch, "%s\n", race_table[race].name);

		stc("\n{CWhat is your race?{x ", ch);
		d->connected = CON_GET_NEW_RACE;
		break;

	case CON_GET_NEW_RACE:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument[0] == '\0')
				help(ch, "races");
			else
				help(ch, argument);

			write_to_buffer(d, "What is your race? ", 0);
			break;
		}

		if (!(race = race_lookup(argument))
		    || !race_table[race].pc_race
		    || pc_race_table[race].remort_level) {
			write_to_buffer(d, "That is not a valid race.\nWhat is your race? ", 0);
			break;
		}

		ch->race = race;

		/* initialize stats */
		for (i = 0; i < MAX_STATS; i++)
			ch->perm_stat[i] = pc_race_table[race].stats[i];

		ch->affected_by         = ch->affected_by | race_table[race].aff;
		ch->drain_flags         = 0;
		ch->imm_flags           = ch->imm_flags | race_table[race].imm;
		ch->res_flags           = ch->res_flags | race_table[race].res;
		ch->vuln_flags          = ch->vuln_flags | race_table[race].vuln;
		ch->form                = race_table[race].form;
		ch->parts               = race_table[race].parts;
		ch->pcdata->points      = pc_race_table[race].points;
		ch->size                = pc_race_table[race].size;

		/* add skills */
		for (i = 0; i < 5 && pc_race_table[race].skills[i]; i++)
			group_add(ch, pc_race_table[race].skills[i], FALSE);

		write_to_buffer(d, "\n", 2);
		write_to_buffer(d, "Here are your default stats:\n", 0);
		sprintf(buf, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
		        ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
		        ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
		        ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHR]);
		write_to_buffer(d, buf, 0);
		write_to_buffer(d, "Would you like to roll for new stats? [Y/N] ", 0);
		d->connected = CON_ROLL_STATS;
		break;

	case CON_ROLL_STATS:
		switch (argument[0]) {
		case 'n':
		case 'N':
			write_to_buffer(d, "\n", 2);
			write_to_buffer(d, "What is your sex (M/F)? ", 0);
			d->connected = CON_GET_NEW_SEX;
			break;

		case 'y':
		case 'Y':
			for (i = 0; i < 6; i++)
				ch->perm_stat[i] = roll_stat(ch, i);

			sprintf(buf, "\nStr: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
			        ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
			        ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
			        ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHR]);
			write_to_buffer(d, buf, 0);
			write_to_buffer(d, "Would you like to roll for new stats? [Y/N] ", 0);
			d->connected = CON_ROLL_STATS;
			break;

		default:
			write_to_buffer(d, "Yes or No? ", 0);
			break;
		}

		break;

	case CON_GET_NEW_SEX:
		switch (argument[0]) {
		case 'm':
		case 'M':
			ch->sex = ch->pcdata->true_sex = SEX_MALE;
			break;

		case 'f':
		case 'F':
			ch->sex = ch->pcdata->true_sex = SEX_FEMALE;
			break;

		default:
			write_to_buffer(d, "That's not a sex.\n(M/F)? ", 0);
			return;
		}

		write_to_buffer(d, "\n", 2);
		strcpy(buf, "Select a class [");

		for (iClass = 0; iClass < MAX_CLASS; iClass++) {
			if (iClass > 0)
				strcat(buf, " ");

			strcat(buf, class_table[iClass].name);
		}

		strcat(buf, "]\nHelp file: class\nWhat is your class? ");
		write_to_buffer(d, buf, 0);
		d->connected = CON_GET_NEW_CLASS;
		break;

	case CON_GET_NEW_CLASS:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument[0] == '\0')
				help(ch, "class help");
			else
				help(ch, argument);

			write_to_buffer(d, "What is your class? ", 0);
			break;
		}

		if ((iClass = class_lookup(argument)) == -1) {
			write_to_buffer(d, "That is not a class.\nWhat is your class? ", 0);
			return;
		}

		ch->class = iClass;
		sprintf(log_buf, "%s@%s new player.", ch->name, d->host);
		wiznet(log_buf, NULL, NULL, WIZ_LOGINS, 0, GET_RANK(ch));
		log_string(log_buf);
		sprintf(log_buf, "Newbie alert!  %s sighted.", ch->name);
		wiznet(log_buf, ch, NULL, WIZ_NEWBIE, 0, 0);
		write_to_buffer(d, "\n", 2);
		/* paladins can't be neutral */
		sprintf(buf, "You may be good%s or evil.\nWhich alignment (G%s/E)? ",
		        ch->class == PALADIN_CLASS ? "" : ", neutral,",
		        ch->class == PALADIN_CLASS ? "" : "/N");
		write_to_buffer(d, buf, 0);
		d->connected = CON_GET_ALIGNMENT;
		break;

	case CON_GET_ALIGNMENT:
		switch (argument[0]) {
		case 'g':
		case 'G':
			if (ch->class == PALADIN_CLASS)
				ch->alignment = 1000;
			else
				ch->alignment = 750;

			break;

		case 'e':
		case 'E':
			if (ch->class == PALADIN_CLASS)
				ch->alignment = -1000;
			else
				ch->alignment = -750;

			break;

		case 'n':
		case 'N':

			/* paladins drop through to default */
			if (ch->class != PALADIN_CLASS) {
				ch->alignment = 0;
				break;
			}

		default:
			sprintf(buf, "That's not a valid alignment.\nWhich alignment (G%s/E)? ",
			        ch->class == PALADIN_CLASS ? "" : "/N");
			write_to_buffer(d, buf, 0);
			return;
		}

		write_to_buffer(d, "\n", 0);
		group_add(ch, "rom basics", FALSE);
		group_add(ch, class_table[ch->class].base_group, FALSE);
		ch->pcdata->learned[gsn_recall] = 50;
		ch->pcdata->learned[gsn_scan]   = 100;
		strcpy(buf, "Select a deity:\n");

		for (deity = 0; deity_table[deity].name != NULL; deity++) {
			if (ch->class == PALADIN_CLASS) { /* Paladins */
				if (deity_table[deity].value > 0 && ch->alignment > 0) {
					strcat(buf, deity_table[deity].align);
					strcat(buf, deity_table[deity].name);
					strcat(buf, "\n");
				}
				else if (deity_table[deity].value < 0 && ch->alignment < 0) {
					strcat(buf, deity_table[deity].align);
					strcat(buf, deity_table[deity].name);
					strcat(buf, "\n");
				}
			}
			else if (deity_table[deity].value == ch->alignment
			         ||       deity_table[deity].value == -1) {
				strcat(buf, deity_table[deity].align);
				strcat(buf, deity_table[deity].name);
				strcat(buf, "\n");
			}
		}

		write_to_buffer(d, buf, 0);
		write_to_buffer(d, "\nHelp file: deity\nWho is your deity? ", 0);
		d->connected = CON_DEITY;
		break;

	case CON_DEITY:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument[0] == '\0')
				help(ch, "deity");
			else
				help(ch, argument);

			write_to_buffer(d, "Who is your deity? ", 0);
			break;
		}

		if ((deity = deity_lookup(argument)) == -1) {
			write_to_buffer(d, "That's not a valid deity.\nWho is your deity? ", 0);
			return;
		}

		free_string(ch->pcdata->deity);
		ch->pcdata->deity = str_dup(deity_table[deity].name);
		write_to_buffer(d, "\nDo you wish to customize this character?\n", 0);
		write_to_buffer(d, "Customization takes time, but allows a wider range of skills"
		                " and abilities.\nCustomize (Y/N)? ", 0);
		d->connected = CON_DEFAULT_CHOICE;
		break;

	case CON_DEFAULT_CHOICE:
		switch (argument[0]) {
		case 'y':
		case 'Y':
			ch->gen_data = new_gen_data();
			ch->gen_data->points_chosen = ch->pcdata->points;
			help(ch, "group header");
			list_group_costs(ch);
			write_to_buffer(d, "You already have the following skills:\n", 0);
			do_skills(ch, "");
			help(ch, "menu choice");
			d->connected = CON_GEN_GROUPS;
			break;

		case 'n':
		case 'N':
			group_add(ch, class_table[ch->class].default_group, TRUE);
			write_to_buffer(d, "\n", 2);
			write_to_buffer(d, "Please pick a weapon from the following choices:\n", 0);
			buf[0] = '\0';

			for (i = 0; weapon_table[i].name != NULL; i++)
				if (ch->pcdata->learned[*weapon_table[i].gsn] > 0) {
					strcat(buf, weapon_table[i].name);
					strcat(buf, " ");
				}

			strcat(buf, "\nYour choice? ");
			write_to_buffer(d, buf, 0);
			d->connected = CON_PICK_WEAPON;
			break;

		default:
			write_to_buffer(d, "Please answer (Y/N)? ", 0);
			return;
		}

		break;

	case CON_PICK_WEAPON:
		write_to_buffer(d, "\n", 2);
		weapon = weapon_lookup(argument);

		if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0) {
			write_to_buffer(d, "That is not a valid selection.  You choice? \n", 0);
			return;
		}

		ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
		write_to_buffer(d, "\n", 2);
		set_color(ch, CYAN, BOLD);
		help(ch, "automotd");
		d->connected = CON_READ_MOTD;
		set_color(ch, WHITE, NOBOLD);
		break;

	case CON_GEN_GROUPS:
		stc("\n", ch);

		if (!str_cmp(argument, "done")) {
			if (ch->pcdata->points > 300)
				ch->pcdata->points = 300;

			ptc(ch, "Creation points: %d\n", ch->pcdata->points);
			ptc(ch, "Experience per level: %ld\n", exp_per_level(ch, ch->gen_data->points_chosen));

			if (ch->pcdata->points < 40)
				ch->train = (40 - ch->pcdata->points + 1) / 2;

			free_gen_data(ch->gen_data);
			ch->gen_data = NULL;
			write_to_buffer(d, "\n", 2);
			write_to_buffer(d, "Please pick a weapon from the following choices:\n", 0);
			buf[0] = '\0';

			for (i = 0; weapon_table[i].name != NULL; i++)
				if (ch->pcdata->learned[*weapon_table[i].gsn] > 0) {
					strcat(buf, weapon_table[i].name);
					strcat(buf, " ");
				}

			strcat(buf, "\nYour choice? ");
			write_to_buffer(d, buf, 0);
			d->connected = CON_PICK_WEAPON;
			break;
		}

		if (!parse_gen_groups(ch, argument))
			stc("Choices are: list, learned, premise, add, drop, info, help, and done.\n", ch);

		help(ch, "menu choice");
		break;

	case CON_READ_IMOTD:
		write_to_buffer(d, "\n", 2);
		set_color(ch, CYAN, NOBOLD);
		help(ch, "automotd");
		d->connected = CON_READ_MOTD;
		set_color(ch, WHITE, NOBOLD);
		break;

	case CON_READ_MOTD:
		set_color(ch, WHITE, BOLD);

		if (time_info.motd != NULL)
			stc(time_info.motd, ch);

		stc("\n{x[Hit Enter to continue]", ch);
		d->connected = CON_READ_NEWMOTD;
		set_color(ch, WHITE, NOBOLD);
		break;

	case CON_READ_NEWMOTD:
		set_color(ch, WHITE, NOBOLD);

		if (ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0') {
			write_to_buffer(d, "Warning! Null password!\n", 0);
			write_to_buffer(d, "Please report old password with 'bug'.\n", 0);
			write_to_buffer(d, "Type 'password null <new password>' to fix.\n", 0);
		}

		ch->next                = char_list;
		char_list               = ch;
		ch->pcdata->next        = pc_list;
		pc_list                 = ch->pcdata;
		d->connected = CON_PLAYING;

		if (ch->level == 0) {
			OBJ_DATA *obj;   /* a generic object variable */
			ch->perm_stat[class_table[ch->class].attr_prime] += 3;
			ch->level       = 1;
			ch->exp         = exp_per_level(ch, ch->pcdata->points);
			ch->hit         = ch->max_hit;
			ch->mana        = ch->max_mana;
			ch->stam        = ch->max_stam;
			ch->train       = 3;
			ch->practice    = 5;
			sprintf(buf, "({VNewbie Aura{x)");
			set_title(ch, buf);
			do_outfit(ch, "");
			/* This is ugly and doesn't error check. -- Outsider
			   I'm re-writing it underneath.
			       obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP), 0), ch);
			       obj_to_char(create_object(get_obj_index(OBJ_VNUM_TOKEN), 0), ch);
			*/
			obj = create_object(get_obj_index(OBJ_VNUM_MAP), 0);

			if (! obj)
				bug("Error creating object in nanny.c", 0);
			else
				obj_to_char(obj, ch);

			obj = create_object(get_obj_index(OBJ_VNUM_TOKEN), 0);

			if (! obj)
				bug("Error creating object in nanny.c", 0);
			else
				obj_to_char(obj, ch);

			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
			stc("\n", ch);
			set_color(ch, PURPLE, BOLD);
			help(ch, "NEWBIE INFO");
			set_color(ch, WHITE, NOBOLD);
			stc("\n", ch);
		}
		else if (ch->in_room != NULL)
			char_to_room(ch, ch->in_room);
		else if (IS_IMMORTAL(ch))
			char_to_room(ch, get_room_index(ROOM_VNUM_CHAT));
		else
			char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));

		if (ch->pcdata->email != NULL && ch->pcdata->email[0] == '\0') {
			set_color(ch, RED, BOLD);
			stc("Your e-mail has not been set, please update your email address\n", ch);
			stc("with the email command!\n\n", ch);
			set_color(ch, WHITE, NOBOLD);
		}

		for (sd = descriptor_list; sd != NULL; sd = sd->next) {
			victim = sd->original ? sd->original : sd->character;

			if (IS_PLAYING(sd)
			    && sd->character != ch
			    && can_see_who(victim, ch)
			    && !IS_SET(victim->comm, COMM_NOANNOUNCE)
			    && !IS_SET(victim->comm, COMM_QUIET)) {
				if (ch->pcdata && ch->pcdata->gamein && ch->pcdata->gamein[0]) {
					set_color(victim, GREEN, BOLD);
					ptc(victim, "[%s] %s\n", ch->name, ch->pcdata->gamein);
				}
				else {
					new_color(victim, CSLOT_CHAN_ANNOUNCE);
					ptc(victim, "[FYI] %s has entered the game.\n", ch->name);
				}

				set_color(victim, WHITE, NOBOLD);
			}
		}

		do_look(ch, "auto");
		wiznet("$N has left real life behind.", ch, NULL, WIZ_LOGINS, WIZ_SITES, GET_RANK(ch));

		if (ch->pet != NULL) {
			char_to_room(ch->pet, ch->in_room);
			act("$n has entered the game.", ch->pet, NULL, NULL, TO_ROOM);
		}

		do_unread(ch, "");
		stc("\n", ch);

		if (!IS_SET(ch->censor, CENSOR_CHAN))
			stc("{BL{Ce{gg{Wa{Cc{By{x is currently rated {PR{x.\n", ch);
		else
			stc("{BL{Ce{gg{Wa{Cc{By{x is currently rated {GPG{x.\n", ch);

		if (get_play_seconds(ch) - ch->pcdata->backup >= 3600) {
			int last = (get_play_seconds(ch) - ch->pcdata->backup) / 3600;

			if (ch->pcdata->backup == 0)
				stc("{W{fThere is currently no backup for your character.{x\n", ch);
			else
				ptc(ch, "%sYou have not backed up for %s%d%s hour%s of gameplay.{x\n",
				    last > 24 ? "{P" : "{W",
				    last > 24 ? "{V" : "{G",
				    last,
				    last > 24 ? "{P" : "{W",
				    last > 1  ? "s"  : "");
		}

		ptc(ch, "\nYour last login was from [{W%s{x] %s",
		    ch->pcdata->last_lsite[0] != '\0' ? ch->pcdata->last_lsite : "Not Available",
		    ch->pcdata->last_ltime != (time_t) 0 ? dizzy_ctime(&ch->pcdata->last_ltime) : "00:00:00");
		ch->pcdata->last_ltime = current_time;
		ch->pcdata->last_lsite = str_dup(d->host);
		ptc(ch, "\n{VYou are traveler [%lu] of Legacy!!!{x\n", update_records());
		update_pc_index(ch, FALSE);

		/* VT100 Stuff */
		if (ch->pcdata && IS_SET(ch->pcdata->video, PLR_VT100)) {
			goto_line(ch, 1, 1);
			stc(VT_SETWIN_CLEAR, ch);
			set_window(ch, 1, ch->lines - 2);
			goto_line(ch, ch->lines, 1);
		}

		break;
	}
}

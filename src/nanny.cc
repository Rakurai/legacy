#include <vector>

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "Player.hh"
#include "Room.hh"
#include "sql.hh"
#include "String.hh"
#include "telnet.hh"
#include "vt100.hh"

extern bool    check_playing           args((Descriptor *d, const String& name));
int     roll_stat               args((Character *ch, int stat));

extern void     roll_raffects   args((Character *ch, Character *victim));
extern void     goto_line    args((Character *ch, int row, int column));
extern void     set_window   args((Character *ch, int top, int bottom));

bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */

/*
 * Socket and TCP/IP stuff.
 */
const   unsigned char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   unsigned char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
//const   unsigned char    go_ahead_str    [] = { IAC, GA, '\0' };

/**
 * check_ban
 * Run from nanny() to check if a site is banned.
 */
bool check_ban(const String& site, int type)
{
	bool ban = FALSE;

	if (db_queryf("check_ban", "SELECT flags, site FROM bans WHERE ((flags>>2)<<2)=%d", type) != SQL_OK)
		return FALSE;

	while (!ban && db_next_row() == SQL_OK) {
		Flags flags = db_get_column_flags(0);
		String str = db_get_column_str(1);
		bool prefix = flags.has(BAN_PREFIX);
		bool suffix = flags.has(BAN_SUFFIX);

		if ((prefix  &&  suffix && str.is_infix_of(site))
		    || (prefix  && !suffix && str.is_suffix_of(site))
		    || (!prefix &&  suffix && str.is_prefix_of(site))
		    || (!prefix && !suffix && site == str))
			ban = TRUE;
	}

	return ban;
}

bool check_deny(const String& name)
{
	if (db_countf("check_deny", "SELECT COUNT(*) FROM denies WHERE name='%s'", name) <= 0)
		return FALSE;

	return TRUE;
}

bool check_player_exist(Descriptor *d, const String& name)
{
	Descriptor *dold;
	StoredPlayer *exist = nullptr;    /* is character in storage */

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		    &&   dold->character != nullptr
		    &&   dold->character->level < 1
		    &&   dold->connected != CON_PLAYING
		    &&   name == (dold->original
		                  ? dold->original->name : dold->character->name)) {
			write_to_buffer(d,
			                "A character by that name is currently being created.\n"
			                "You cannot access that character.\n"
			                "Please create a character with a different name, and\n"
			                "ask an Immortal for help if you need it.\n"
			                "\n"
			                "Name: ");
			d->connected = CON_GET_NAME;
			return TRUE;
		}
	}

	/* make sure that we do not re-create a character currently
	   in storage -- Outsider <slicer69@hotmail.com>
	*/
	/* first make sure we have the list of stored characters */
	if (! storage_list_head)
		load_storage_list();

	/* search storage for character name */
	exist = lookup_storage_data(name);

	if (exist) {
		write_to_buffer(d,
		                "A character by that name is currently in storage.\n"
		                "You cannot create a character by this name.\n"
		                "Please create a character with a different name, and\n"
		                "ask an Immortal for help if you need it.\n"
		                "\n"
		                "Name: ");
		d->connected = CON_GET_NAME;
		return TRUE;
	}

	return FALSE;
}

String site_to_ssite(const String& site)
{
	bool alpha = FALSE;
	int dotcount = 0;

	/* Parse the site, determine type.  For alphanumeric hosts, we
	   match the last three dot sections, for straight numerics we
	   match the first three. */
	for (const char *p = site.c_str(); *p; p++) {
		if (*p == '.')
			dotcount++;
		else if (!isdigit(*p))
			alpha = TRUE;
	}

	if (alpha) {
		if (dotcount < 3)
			return site;

		return site.substr(site.find_nth(dotcount - 2, "."));
	}

	return site.substr(0, site.find_nth(3, "."));
}

void update_site(Character *ch)
{
	String shortsite = site_to_ssite(ch->desc->host);

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
	Descriptor *d;
	int count = 0;

	if (Game::port != DIZZYPORT)
		return 0;

	db_command("update_records", "UPDATE records SET logins=logins+1");

	for (d = descriptor_list; d != nullptr; d = d->next)
		if (IS_PLAYING(d))
			count++;

	Game::record_players_since_boot = UMAX(count, Game::record_players_since_boot);

	if (Game::record_players_since_boot > Game::record_players) {
		Game::record_players = Game::record_players_since_boot;
		db_commandf("update_records", "UPDATE records SET players=%d", Game::record_players_since_boot);
	}

	return ++Game::record_logins;
}

void update_pc_index(Character *ch, bool remove)
{
	db_commandf("update_pc_index", "DELETE FROM pc_index WHERE name='%s'", db_esc(ch->name));

	if (!remove)
		db_commandf("update_pc_index",
		            "INSERT INTO pc_index VALUES('%s','%s','%s','%s',%ld,%d,%d,'%s','%s')",
		            db_esc(ch->name),
		            db_esc(ch->pcdata->title),
		            db_esc(ch->pcdata->deity),
		            db_esc(ch->pcdata->deity.uncolor()),
		            ch->pcdata->cgroup_flags.to_ulong(),
		            ch->level,
		            ch->pcdata->remort_count,
		            ch->clan ? db_esc(ch->clan->name) : "",
		            ch->clan && !ch->pcdata->rank.empty() ? db_esc(ch->pcdata->rank) : "");
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name(const String& name)
{
	Clan *clan;

	/*
	 * Reserved words.
	 */
	if (String(
		"all auto immortal self remort imms private someone something the you"
		).has_words(name))
		return FALSE;

	if ((clan = clan_lookup(name)) != nullptr)
		return FALSE;

	/*
	 * Length restrictions.
	 */

	if (strlen(name) <  2)
		return FALSE;

	if (strlen(name) > 12)
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		bool fIll, adjcaps = FALSE, cleancaps = FALSE;
		unsigned int total_caps = 0;
		fIll = TRUE;

		for (const char *pc = name.c_str(); *pc != '\0'; pc++) {
			if (!isalpha(*pc))
				return FALSE;

			if (isupper(*pc)) { /* ugly anti-caps hack */
				if (adjcaps)
					cleancaps = TRUE;

				total_caps++;
				adjcaps = TRUE;
			}
			else
				adjcaps = FALSE;

			if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
				fIll = FALSE;
		}

		if (fIll)
			return FALSE;

		if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
			return FALSE;
	}

	return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(Descriptor *d, const String& name, bool fConn)
{
	Character *ch;
	Room *room;

	for (ch = Game::world().char_list; ch != nullptr; ch = ch->next) {
		if (!IS_NPC(ch)
		    && d->character != ch
		    && (!fConn || ch->desc == nullptr)
		    &&   d->character->name == ch->name) {
			if (fConn == FALSE) {
				d->character->pcdata->pwd = ch->pcdata->pwd;
			}
			else {
				Character *rch;
				delete d->character;
				d->character = ch;
				ch->desc         = d;
				ch->desc->timer  = 0;
				stc("Reconnecting...\n", ch);

				if (!IS_NPC(ch))
					if (!ch->pcdata->buffer.empty())
						stc("You have messages: Type 'replay'\n", ch);

				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					if (ch != rch && can_see_char(rch, ch))
						ptc(rch, "%s has reconnected.\n", PERS(ch, rch, VIS_CHAR));

				Logging::logf("%s@%s reconnected.", ch->name, d->host);
				wiznet("$N reclaims the fullness of $S link.",
				       ch, nullptr, WIZ_LINKS, 0, 0);

				if ((room = ch->in_room) != nullptr) {
					char_from_room(ch);
					char_to_room(ch, room);
				}

				ch->pcdata->plr_flags -= PLR_LINK_DEAD;
				d->connected = CON_PLAYING;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(Descriptor *d, String argument)
{
	Descriptor *d_old, *d_next, *sd;
	String buf, arg;
	Character *ch, *victim;
	const char *pwdnew, *p;
	Guild iGuild; 
	int i, weapon, deity;
	unsigned int race;
	bool fOld, logon_lurk;
	String log_buf;

	argument = argument.lstrip();

	ch = d->character;

	switch (d->connected) {
	default:
		Logging::bug("Nanny: bad d->connected %d.", d->connected);
		close_socket(d);
		return;

	case CON_GET_NAME:
		if (argument.empty()) {
			close_socket(d);
			return;
		}

		logon_lurk = FALSE;

		if (argument[0] == '-') { /* Lurk mode -- Elrac */
			logon_lurk = TRUE;
			argument.erase(0, 1);
		}

		char name[MIL];
		strcpy(name, argument);
		name[0] = UPPER(name[0]);

		/* Check valid name - Lotus */
		if (!check_parse_name(name)) {
			write_to_buffer(d, "Sorry, that name cannot be used.\n"
			                "Please choose another name!\n"
			                "\n"
			                "Name: ");
			delete d->character;
			d->character = nullptr;
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
			                "Name: ");
			delete ch;
			d->character = nullptr;
			return;
		}

		if (check_deny(ch->name)) {
			Logging::logf("Denying access to %s@%s.", ch->name, d->host);
			write_to_buffer(d, "You are denied access to Legacy.\n");
			close_socket(d);
			return;
		}

		if (check_ban(d->host, BAN_ALL) && !ch->act_flags.has(PLR_PERMIT)) {
			log_buf = Format::format("Disconnecting because BANned: %s", d->host);
			Logging::log(log_buf);
			wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, 0);
			write_to_buffer(d, "Your site has been banned from this mud.\n"
			                "If you feel that your site has been banned in error, or would\n"
			                "like to request special permission to play, please contact us at:\n"
			                "   legacy@kyndig.com\n");
			close_socket(d);
			return;
		}

		if (check_reconnect(d, name, FALSE))
			fOld = TRUE;
		else if (wizlock && !IS_IMMORTAL(ch)) {
			write_to_buffer(d, "Access has been limited to imms only at this time.\n");
			close_socket(d);
			return;
		}

		if (fOld) { /* old player */
			if (logon_lurk && IS_IMMORTAL(ch))
				ch->lurk_level = LEVEL_IMMORTAL;

			write_to_buffer(d, "What is your password? ");
			write_to_buffer(d, (const char *)echo_off_str);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}

		if (newlock) {
			write_to_buffer(d, "Due to technical difficulties, we are not accepting new players\n"
			                "at this time.  Please try again in a few hours.\n");
			close_socket(d);
			return;
		}

		if (check_ban(d->host, BAN_NEWBIES)) {
			log_buf = Format::format("Disconnecting because NewbieBANned: %s", d->host);
			Logging::log(log_buf);
			wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, 0);
			write_to_buffer(d, "New players are not allowed from your site.\n"
			                "If you feel that your site has been banned in error, or would\n"
			                "like to request special permission to play, please contact us at:\n"
			                "   legacyimms@kyndig.com\n");
			close_socket(d);
			return;
		}

		Format::sprintf(buf, "You wish for history to remember you as %s (Y/N)? ", name);
		write_to_buffer(d, buf);
		d->connected = CON_CONFIRM_NEW_NAME;
		return;

	case CON_GET_OLD_PASSWORD:
		write_to_buffer(d, "\n");

		if (strcmp(argument, ch->pcdata->pwd)) {
			stc("{bIncorrect password!{x\n", ch);
			close_socket(d);
			return;
		}

		write_to_buffer(d, (const char *)echo_on_str);

		if (check_playing(d, ch->name))
			return;

		if (check_reconnect(d, ch->name, TRUE))
			return;

		ch->pcdata->plr_flags -= PLR_LINK_DEAD;
		quest_cleanup(ch);
		sq_cleanup(ch);

		log_buf = Format::format("%s@%s has connected.", ch->name, d->host);
		Logging::log(log_buf);
		wiznet(log_buf, nullptr, nullptr, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));

		log_buf = Format::format("Last Site: %s",
		        !ch->pcdata->last_lsite.empty() ? ch->pcdata->last_lsite : "Not Available");
		wiznet(log_buf, nullptr, nullptr, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));

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
		switch (argument[0]) {
		case 'y':
		case 'Y':
			for (d_old = descriptor_list; d_old != nullptr; d_old = d_next) {
				d_next = d_old->next;

				if (d_old == d || d_old->character == nullptr)
					continue;

				if (ch->name != (d_old->original ?
				            d_old->original->name : d_old->character->name))
					continue;

				close_socket(d_old);
			}

			if (check_reconnect(d, ch->name, TRUE))
				return;

			write_to_buffer(d, "Reconnect attempt failed.\nName: ");

			if (d->character != nullptr) {
				delete d->character;
				d->character = nullptr;
			}

			d->connected = CON_GET_NAME;
			break;

		case 'n':
		case 'N':
			write_to_buffer(d, "Name: ");

			if (d->character != nullptr) {
				delete d->character;
				d->character = nullptr;
			}

			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Y or N? ");
			break;
		}

		break;

	case CON_CONFIRM_NEW_NAME:

		/* We're past all the reconnect stuff, we can be sure that the character
		   is not playing and is new.  Start using stc and ptc for the colors */
		switch (argument[0]) {
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
			delete d->character;
			d->character = nullptr;
			d->connected = CON_GET_NAME;
			break;

		default:
			stc("Please type Yes or No? ", ch);
			break;
		}

		break;

	case CON_GET_MUD_EXP:
		switch (argument[0]) {
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
		write_to_buffer(d, "\n");

		if (strlen(argument) < 5) {
			stc("You whisper in her ear, and she giggles.\n\n"
			    "{Y'{WThat's too short, somebody could guess it easily.  Make up one that's at"
			    " least five letters long!{Y'{x she says, and flies up close to hear you whisper.\n\n", ch);
			stc("{CWhat password do you tell her?{x ", ch);
			return;
		}

		pwdnew = argument.c_str();

		for (p = pwdnew; *p != '\0'; p++)
			if (*p == '~') {
				stc("She frowns as you whisper in her ear.\n\n"
				    "{Y'{WI'm not sure how you managed to pronounce that, but I'm sure the guards\n"
				    " would not recognize it.  Try something with only letters and numbers, maybe?{Y'{x\n"
				    "She leans close for you to whisper another password.\n\n", ch);
				stc("{CWhat password do you tell her?{x ", ch);
				return;
			}

		ch->pcdata->pwd = pwdnew;
		stc("The pixie smiles as you whisper in her ear.\n\n"
		    "{Y'{WGood, that will do nicely.  Just to make sure I have it, say it one more time.{Y'{x\n\n", ch);
		stc("{CPlease repeat the password.{x ", ch);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
		write_to_buffer(d, "\n");

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

		for (race = 1; race < race_table.size() && race_table[race].pc_race; race++)
			if (!pc_race_table[race].remort_level)
				ptc(ch, "%s\n", race_table[race].name);

		stc("\n{CWhat is your race?{x ", ch);
		d->connected = CON_GET_NEW_RACE;
		break;

	case CON_GET_NEW_RACE:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument.empty())
				help(ch, "races");
			else
				help(ch, argument);

			write_to_buffer(d, "What is your race? ");
			break;
		}

		if (!(race = race_lookup(argument))
		    || !race_table[race].pc_race
		    || pc_race_table[race].remort_level) {
			write_to_buffer(d, "That is not a valid race.\nWhat is your race? ");
			break;
		}

		ch->race = race;

		/* initialize stats */
		for (int stat = 0; stat < MAX_STATS; stat++)
			ATTR_BASE(ch, stat_to_attr(stat)) = pc_race_table[race].stats[stat];

		affect::add_racial_to_char(ch);

		ch->form_flags                = race_table[race].form;
		ch->parts_flags               = race_table[race].parts;
		ch->pcdata->points      = pc_race_table[race].points;
		ch->size                = pc_race_table[race].size;

		/* add skills */
		for (i = 0; i < 5 && !pc_race_table[race].skills[i].empty(); i++)
			group_add(ch, pc_race_table[race].skills[i], FALSE);

		write_to_buffer(d, "\n");
		write_to_buffer(d, "Here are your default stats:\n");
		Format::sprintf(buf, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
		        ATTR_BASE(ch, APPLY_STR), ATTR_BASE(ch, APPLY_INT),
		        ATTR_BASE(ch, APPLY_WIS), ATTR_BASE(ch, APPLY_DEX),
		        ATTR_BASE(ch, APPLY_CON), ATTR_BASE(ch, APPLY_CHR));
		write_to_buffer(d, buf);
		write_to_buffer(d, "Would you like to roll for new stats? [Y/N] ");
		d->connected = CON_ROLL_STATS;
		break;

	case CON_ROLL_STATS:
		switch (argument[0]) {
		case 'n':
		case 'N':
			write_to_buffer(d, "\n");
			write_to_buffer(d, "What is your sex (M/F)? ");
			d->connected = CON_GET_NEW_SEX;
			break;

		case 'y':
		case 'Y':
			for (int stat = 0; stat < MAX_STATS; stat++)
				ATTR_BASE(ch, stat_to_attr(stat)) = roll_stat(ch, stat);

			Format::sprintf(buf, "\nStr: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Chr: %d\n",
			        ATTR_BASE(ch, APPLY_STR), ATTR_BASE(ch, APPLY_INT),
			        ATTR_BASE(ch, APPLY_WIS), ATTR_BASE(ch, APPLY_DEX),
			        ATTR_BASE(ch, APPLY_CON), ATTR_BASE(ch, APPLY_CHR));
			write_to_buffer(d, buf);
			write_to_buffer(d, "Would you like to roll for new stats? [Y/N] ");
			d->connected = CON_ROLL_STATS;
			break;

		default:
			write_to_buffer(d, "Yes or No? ");
			break;
		}

		break;

	case CON_GET_NEW_SEX:
		switch (argument[0]) {
		case 'm':
		case 'M':
			ATTR_BASE(ch, APPLY_SEX) = SEX_MALE;
			break;

		case 'f':
		case 'F':
			ATTR_BASE(ch, APPLY_SEX) = SEX_FEMALE;
			break;

		default:
			write_to_buffer(d, "That's not a sex.\n(M/F)? ");
			return;
		}

		write_to_buffer(d, "\n");
		buf = "Select a class [";

		for (int i = Guild::first; i < Guild::size; i++) {
			if (i > Guild::first)
				buf += " ";

			buf += guild_table[i].name;
		}

		buf += "]\nHelp file: class\nWhat is your class? ";
		write_to_buffer(d, buf);
		d->connected = CON_GET_NEW_CLASS;
		break;

	case CON_GET_NEW_CLASS:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument.empty())
				help(ch, "class help");
			else
				help(ch, argument);

			write_to_buffer(d, "What is your class? ");
			break;
		}

		if ((iGuild = guild_lookup(argument)) == Guild::none) {
			write_to_buffer(d, "That is not a class.\nWhat is your class? ");
			return;
		}

		ch->guild = iGuild;
		log_buf = Format::format("%s@%s new player.", ch->name, d->host);
		wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, GET_RANK(ch));
		Logging::log(log_buf);
		log_buf = Format::format("Newbie alert!  %s sighted.", ch->name);
		wiznet(log_buf, ch, nullptr, WIZ_NEWBIE, 0, 0);
		write_to_buffer(d, "\n");
		/* paladins can't be neutral */
		Format::sprintf(buf, "You may be good%s or evil.\nWhich alignment (G%s/E)? ",
		        ch->guild == Guild::paladin ? "" : ", neutral,",
		        ch->guild == Guild::paladin ? "" : "/N");
		write_to_buffer(d, buf);
		d->connected = CON_GET_ALIGNMENT;
		break;

	case CON_GET_ALIGNMENT:
		switch (argument[0]) {
		case 'g':
		case 'G':
			if (ch->guild == Guild::paladin)
				ch->alignment = 1000;
			else
				ch->alignment = 750;

			break;

		case 'e':
		case 'E':
			if (ch->guild == Guild::paladin)
				ch->alignment = -1000;
			else
				ch->alignment = -750;

			break;

		case 'n':
		case 'N':

			/* paladins drop through to default */
			if (ch->guild != Guild::paladin) {
				ch->alignment = 0;
				break;
			}

		default:
			Format::sprintf(buf, "That's not a valid alignment.\nWhich alignment (G%s/E)? ",
			        ch->guild == Guild::paladin ? "" : "/N");
			write_to_buffer(d, buf);
			return;
		}

		write_to_buffer(d, "\n");
		group_add(ch, "rom basics", FALSE);
		group_add(ch, guild_table[ch->guild].base_group, FALSE);
		set_learned(ch, skill::type::recall, 50);
		set_learned(ch, skill::type::scan, 100);
		buf = "Select a deity:\n";

		for (const auto& entry : deity_table) {
			if (ch->guild == Guild::paladin) { /* Paladins */
				if (entry.value > 0 && ch->alignment > 0) {
					buf += entry.align;
					buf += entry.name;
					buf += "\n";
				}
				else if (entry.value < 0 && ch->alignment < 0) {
					buf += entry.align;
					buf += entry.name;
					buf += "\n";
				}
			}
			else if (entry.value == ch->alignment
			         ||       entry.value == -1) {
				buf += entry.align;
				buf += entry.name;
				buf += "\n";
			}
		}

		write_to_buffer(d, buf);
		write_to_buffer(d, "\nHelp file: deity\nWho is your deity? ");
		d->connected = CON_DEITY;
		break;

	case CON_DEITY:
		one_argument(argument, arg);

		if (!strcmp(arg, "help")) {
			argument = one_argument(argument, arg);

			if (argument.empty())
				help(ch, "deity");
			else
				help(ch, argument);

			write_to_buffer(d, "Who is your deity? ");
			break;
		}

		if ((deity = deity_lookup(argument)) == -1) {
			write_to_buffer(d, "That's not a valid deity.\nWho is your deity? ");
			return;
		}

		ch->pcdata->deity = deity_table[deity].name;
//		write_to_buffer(d, "\nDo you wish to customize this character?\n", 0);
//		write_to_buffer(d, "Customization takes time, but allows a wider range of skills"
//		                " and abilities.\nCustomize (Y/N)? ", 0);
		d->connected = CON_DEFAULT_CHOICE;
//		break;

	case CON_DEFAULT_CHOICE:
/*		switch (argument[0]) {
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
*/
//			group_add(ch, guild_table[ch->guild].default_group, TRUE);

			if (ch->pcdata->points < 40)
				ch->train = 40 - ch->pcdata->points;

			write_to_buffer(d, "\n");
			write_to_buffer(d, "Please pick a weapon from the following choices:\n");
			buf.clear();

			for (const auto& entry : weapon_table)
				if (get_learned(ch, entry.skill) > 0) {
					buf += entry.name;
					buf += " ";
				}

			buf += "\nYour choice? ";
			write_to_buffer(d, buf);
			d->connected = CON_PICK_WEAPON;
/*			break;

		default:
			write_to_buffer(d, "Please answer (Y/N)? ", 0);
			return;
		}
*/
		break;

	case CON_PICK_WEAPON:
		write_to_buffer(d, "\n");
		weapon = weapon_lookup(argument);

		if (weapon == -1 || get_learned(ch, weapon_table[weapon].skill) <= 0) {
			write_to_buffer(d, "That is not a valid selection.  You choice? \n");
			return;
		}

		set_learned(ch, weapon_table[weapon].skill, 40);
		write_to_buffer(d, "\n");
		set_color(ch, CYAN, BOLD);
		help(ch, "automotd");
		d->connected = CON_READ_MOTD;
		set_color(ch, WHITE, NOBOLD);
		break;
/*
	case CON_GEN_GROUPS:
		stc("\n", ch);

		if (argument == "done") {
			if (ch->pcdata->points > 300)
				ch->pcdata->points = 300;

			ptc(ch, "Creation points: %d\n", ch->pcdata->points);
			ptc(ch, "Experience per level: %ld\n", exp_per_level(ch, ch->gen_data->points_chosen));

			if (ch->pcdata->points < 40)
				ch->train = (40 - ch->pcdata->points + 1) / 2;

			write_to_buffer(d, "\n");
			write_to_buffer(d, "Please pick a weapon from the following choices:\n");
			buf.clear();

			for (i = 0; i < weapon_table.size(); i++)
				if (ch->pcdata->learned[*weapon_table[i].gsn] > 0) {
					buf += weapon_table[i].name;
					buf += " ";
				}

			buf += "\nYour choice? ";
			write_to_buffer(d, buf);
			d->connected = CON_PICK_WEAPON;
			break;
		}

		if (!parse_gen_groups(ch, argument))
			stc("Choices are: list, learned, premise, add, drop, info, help, and done.\n", ch);

		help(ch, "menu choice");
		break;
*/
	case CON_READ_IMOTD:
		write_to_buffer(d, "\n");
		set_color(ch, CYAN, NOBOLD);
		help(ch, "automotd");
		d->connected = CON_READ_MOTD;
		set_color(ch, WHITE, NOBOLD);
		break;

	case CON_READ_MOTD:
		set_color(ch, WHITE, BOLD);

		if (!Game::motd.empty())
			stc(Game::motd, ch);

		stc("\n{x[Hit Enter to continue]", ch);
		d->connected = CON_READ_NEWMOTD;
		set_color(ch, WHITE, NOBOLD);
		break;

	case CON_READ_NEWMOTD:
		set_color(ch, WHITE, NOBOLD);

		if (ch->pcdata == nullptr || ch->pcdata->pwd.empty()) {
			write_to_buffer(d, "Warning! Null password!\n");
			write_to_buffer(d, "Please report old password with 'bug'.\n");
			write_to_buffer(d, "Type 'password null <new password>' to fix.\n");
		}

		ch->next                = Game::world().char_list;
		Game::world().char_list               = ch;
		ch->validate();
		
		ch->pcdata->next        = Game::world().pc_list;
		Game::world().pc_list                 = ch->pcdata;
		d->connected = CON_PLAYING;

		if (ch->level == 0) {
			Object *obj;   /* a generic object variable */
			ATTR_BASE(ch, stat_to_attr(guild_table[ch->guild].stat_prime)) += 3;
			ch->level       = 1;
			ch->exp         = exp_per_level(ch, ch->pcdata->points);
			ch->hit         = GET_MAX_HIT(ch);
			ch->mana        = GET_MAX_MANA(ch);
			ch->stam        = GET_MAX_STAM(ch);
			ch->train       += 3;
			ch->practice    += 5;
			Format::sprintf(buf, "({VNewbie Aura{x)");
			set_title(ch, buf);
			do_outfit(ch, "");
			/* This is ugly and doesn't error check. -- Outsider
			   I'm re-writing it underneath.
			       obj_to_char(create_object(Game::world().get_obj_prototype(OBJ_VNUM_MAP), 0), ch);
			       obj_to_char(create_object(Game::world().get_obj_prototype(OBJ_VNUM_TOKEN), 0), ch);
			*/
			obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_MAP), 0);

			if (! obj)
				Logging::bug("Error creating object in nanny.c", 0);
			else
				obj_to_char(obj, ch);

			obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_TOKEN), 0);

			if (! obj)
				Logging::bug("Error creating object in nanny.c", 0);
			else
				obj_to_char(obj, ch);

			char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_SCHOOL))));
			stc("\n", ch);
			set_color(ch, PURPLE, BOLD);
			help(ch, "NEWBIE INFO");
			set_color(ch, WHITE, NOBOLD);
			stc("\n", ch);
		}
		else if (ch->in_room != nullptr)
			char_to_room(ch, ch->in_room);
		else if (IS_IMMORTAL(ch))
			char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_CHAT))));
		else
			char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_TEMPLE))));

		if (ch->pcdata->email.empty()) {
			set_color(ch, RED, BOLD);
			stc("Your e-mail has not been set, please update your email address\n", ch);
			stc("with the email command!\n\n", ch);
			set_color(ch, WHITE, NOBOLD);
		}

		for (sd = descriptor_list; sd != nullptr; sd = sd->next) {
			victim = sd->original ? sd->original : sd->character;

			if (IS_PLAYING(sd)
			    && sd->character != ch
			    && can_see_who(victim, ch)
			    && !victim->comm_flags.has(COMM_NOANNOUNCE)
			    && !victim->comm_flags.has(COMM_QUIET)) {
				if (ch->pcdata && !ch->pcdata->gamein.empty()) {
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
		wiznet("$N has left real life behind.", ch, nullptr, WIZ_LOGINS, WIZ_SITES, GET_RANK(ch));

		if (ch->pet != nullptr) {
			char_to_room(ch->pet, ch->in_room);
			act("$n has entered the game.", ch->pet, nullptr, nullptr, TO_ROOM);
		}

		do_unread(ch, "");
		stc("\n", ch);

		if (!ch->censor_flags.has(CENSOR_CHAN))
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
		    !ch->pcdata->last_lsite.empty() ? ch->pcdata->last_lsite : "Not Available",
		    ch->pcdata->last_ltime != (time_t) 0 ? dizzy_ctime(&ch->pcdata->last_ltime) : "00:00:00");
		ch->pcdata->last_ltime = Game::current_time;
		ch->pcdata->last_lsite = d->host;
		ptc(ch, "\n{VYou are traveler [%lu] of Legacy!!!{x\n", update_records());
		update_pc_index(ch, FALSE);

		/* VT100 Stuff */
		if (ch->pcdata && ch->pcdata->video_flags.has(PLR_VT100)) {
			goto_line(ch, 1, 1);
			stc(VT_SETWIN_CLEAR, ch);
			set_window(ch, 1, ch->lines - 2);
			goto_line(ch, ch->lines, 1);
		}

		break;
	}
}

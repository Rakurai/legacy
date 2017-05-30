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
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "buffer.h"
#include "Format.hpp"

extern void     channel_who     args((CHAR_DATA *ch, const char *channelname, int
                                      channel, int custom));
bool    check_channel_social    args((CHAR_DATA *ch, int channel,
                                      int custom, const String& command, const String& argument));

void    send_to_query           args((CHAR_DATA *ch, const char *string));
bool    swearcheck              args((const String& argument));
extern bool     is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim);
const char   *makedrunk               args((CHAR_DATA *ch, const char *string));

/* RT code to display channel status */
void do_channels(CHAR_DATA *ch, String argument)
{
	/* lists all channels and their status */
	stc("   channel     status\n", ch);
	stc("----------------------\n", ch);
	new_color(ch, CSLOT_CHAN_GOSSIP);
	stc("gossip         ", ch);

	if (IS_SET(ch->revoke, REVOKE_GOSSIP)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOGOSSIP)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_FLAME);
	stc("flame          ", ch);

	if (IS_SET(ch->revoke, REVOKE_FLAME)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOFLAME)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_QWEST);
	stc("qwest          ", ch);

	if (IS_SET(ch->revoke, REVOKE_QWEST)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOQWEST)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_PRAY);
	stc("pray           ", ch);

	if (IS_SET(ch->revoke, REVOKE_PRAY)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOPRAY)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_AUCTION);
	stc("auction        ", ch);

	if (IS_SET(ch->revoke, REVOKE_AUCTION)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOAUCTION)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	if (is_clan(ch) && (!ch->clan->independent || IS_IMMORTAL(ch))) {
		new_color(ch, CSLOT_CHAN_CLAN);
		stc("Clantalk       ", ch);

		if (IS_SET(ch->revoke, REVOKE_CLAN)) stc("REVOKED\n", ch);
		else if (!IS_SET(ch->comm, COMM_NOCLAN)) stc("ON\n", ch);
		else stc("OFF\n", ch);
	}

	new_color(ch, CSLOT_CHAN_ANNOUNCE);
	stc("announce       ", ch);

	if (!IS_SET(ch->comm, COMM_NOANNOUNCE)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_MUSIC);
	stc("music          ", ch);

	if (IS_SET(ch->revoke, REVOKE_MUSIC)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOMUSIC)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_QA);
	stc("Q/A            ", ch);

	if (IS_SET(ch->revoke, REVOKE_QA)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOQUESTION)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_SOCIAL);
	stc("Social         ", ch);

	if (IS_SET(ch->revoke, REVOKE_SOCIAL)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOSOCIAL)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_IC);
	stc("IC             ", ch);

	if (IS_SET(ch->revoke, REVOKE_IC)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOIC)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_GRATS);
	stc("grats          ", ch);

	if (IS_SET(ch->revoke, REVOKE_GRATS)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOGRATS)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	if (IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_IMM);
		stc("Imm Chat       ", ch);

		if (!IS_SET(ch->comm, COMM_NOWIZ)) stc("ON\n", ch);
		else stc("OFF\n", ch);
	}

	new_color(ch, CSLOT_CHAN_PAGE);
	stc("page           ", ch);

	if (IS_SET(ch->revoke, REVOKE_PAGE)) stc("REVOKED\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOPAGE)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	new_color(ch, CSLOT_CHAN_TELL);
	stc("tell           ", ch);

	if (!IS_SET(ch->comm, COMM_DEAF)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	set_color(ch, WHITE, NOBOLD);
	new_color(ch, CSLOT_CHAN_QTELL);
	stc("qtell          ", ch);

	if (IS_SET(ch->revoke, REVOKE_QTELL)) stc("REVOKED\n\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOQUERY)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	set_color(ch, WHITE, NOBOLD);
	stc("quiet mode     ", ch);

	if (IS_SET(ch->comm, COMM_QUIET)) stc("ON\n", ch);
	else stc("OFF\n", ch);

	/*
	 * Stop the mud from crashing when morphed players use channels
	*/
	if (!IS_NPC(ch)) {
		if (IS_SET(ch->pcdata->plr, PLR_NONOTIFY)) stc("You will not be notified of new notes.\n", ch);
		else stc("You {Wwill{x be notified of new notes.\n", ch);

		if (ch->pcdata->aura[0])
			ptc(ch, "{VAura: (%s{V){x\n", ch->pcdata->aura);
	}

	if (IS_SET(ch->comm, COMM_AFK)) stc("You are AFK.\n", ch);

	if (!IS_NPC(ch)) if (IS_SET(ch->pcdata->plr, PLR_SNOOP_PROOF)) stc("You are immune to Nosy people.\n", ch);

	if (ch->lines != PAGELEN) {
		if (ch->lines)
			ptc(ch, "You display %d lines of scroll.\n", ch->lines);
		else
			stc("Scroll buffering is off.\n", ch);
	}

	ptc(ch, "Your current prompt is: %s\n", ch->prompt[0] ? ch->prompt : "(none)");

	if (IS_SET(ch->revoke, REVOKE_TELL))
		stc("No one wants to listen to you.\n", ch);

	if (IS_SET(ch->revoke, REVOKE_EMOTE))
		stc("You're not feeling very emotional right now.\n", ch);

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS))
		stc("Your mouth seems to be stuck shut.\n", ch);

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY))
		stc("You only feel like flaming.\n", ch);
}

String makedrunk(CHAR_DATA *ch, const String& string)
{
	/* This structure defines all changes for a character */
	static struct struckdrunk drunk[] = {
		{
			3, 10,
			{"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}
		},
		{
			8, 5,
			{"b", "b", "b", "B", "B", "vb"}
		},
		{
			3, 5,
			{"c", "c", "C", "cj", "sj", "zj"}
		},
		{
			5, 2,
			{"d", "d", "D"}
		},
		{
			3, 3,
			{"e", "e", "eh", "E"}
		},
		{
			4, 5,
			{"f", "f", "ff", "fff", "fFf", "F"}
		},
		{
			8, 2,
			{"g", "g", "G"}
		},
		{
			9, 6,
			{"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}
		},
		{
			7, 6,
			{"i", "i", "Iii", "ii", "iI", "Ii", "I"}
		},
		{
			9, 5,
			{"j", "j", "jj", "Jj", "jJ", "J"}
		},
		{
			7, 2,
			{"k", "k", "K"}
		},
		{
			3, 2,
			{"l", "l", "L"}
		},
		{
			5, 8,
			{"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}
		},
		{
			6, 6,
			{"n", "n", "nn", "Nn", "nnn", "nNn", "N"}
		},
		{
			3, 6,
			{"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}
		},
		{
			3, 2,
			{"p", "p", "P"}
		},
		{
			5, 5,
			{"q", "q", "Q", "ku", "ququ", "kukeleku"}
		},
		{
			4, 2,
			{"r", "r", "R"}
		},
		{
			2, 5,
			{"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}
		},
		{
			5, 2,
			{"t", "t", "T"}
		},
		{
			3, 6,
			{"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}
		},
		{
			4, 2,
			{"v", "v", "V"}
		},
		{
			4, 2,
			{"w", "w", "W"}
		},
		{
			5, 6,
			{"x", "x", "X", "ks", "iks", "kz", "xz"}
		},
		{
			3, 2,
			{"y", "y", "Y"}
		},
		{
			2, 9,
			{"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}
		}
	};

	if (IS_NPC(ch))
		return string;

	/* Check how drunk a person is... */
	int drunklevel = ch->pcdata->condition[COND_DRUNK];

	if (drunklevel <= 10)
		return string;

	String buf;

	/* drunk in earnest. mangle his speech. */
	for (auto it = string.begin(); it != string.end(); it++) {
		char temp = toupper(*it);

		if ((temp >= 'A') && (temp <= 'Z')) {
			if (drunklevel > drunk[temp - 'A'].min_drunk_level) {
				int randomnum = number_range(0, drunk[temp - 'A'].number_of_rep);
				buf += drunk[temp - 'A'].replacement[randomnum];
			}
			else
				buf += *it;
		}
		else if ((temp >= '0') && (temp <= '9')) {
			buf += '0' + number_range(0, 9);
		}
		else
			buf += *it;
	}

	return (buf);
} /* end makedrunk() */

/* This sends an ACT-type message to everybody in the game. */
void global_act(CHAR_DATA *ch, const char *message,
                int despite_invis, int color, long nocomm_bits)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    !IS_SET(victim->comm, nocomm_bits) &&
		    (ch == NULL || despite_invis || can_see_who(victim, ch))) {
			set_color(victim, color, BOLD);
			act(message, ch, NULL, d->character, TO_VICT, POS_SLEEPING, FALSE);
			set_color(victim, WHITE, NOBOLD);
		}
	}
} /* end global_act() */

bool swearcheck(const String& argument)
{
	/* For partial matches, cause we do want to trigger dickhead */
	char tobechecked[MSL];
	struct swear_type {
		char *word;
		int level;      /* level of checking -- Montrey
                                   1 - only when surrounded by whitespace
                                   2 - at the beginning of any word */
	};
	/* future:  put in a level 3, to check for *any* instace in a string,
	   inside a word or not.  don't feel like messing with pointers right now */
	static const struct swear_type swear_table [] = {
		{       "ass",          1       },
		{       "asshole",      2       },
		{       "dumbass",      2       },
		{       "bastard",      2       },
		{       "cock",         2       },
		{       "clit",         2       },
		{       "whore",        2       },
		{       "shit",         2       },
		{       "fuck",         2       },
		{       "motherfuck",   2       },
		{       "bitch",        2       },
		{       "pussy",        2       },
		{       "dick",         2       },
		{       "slut",         2       },
		{       NULL,           0       }
	};
	int x;  /* Our lovely counter */
	Format::sprintf(tobechecked, "%s", smash_bracket(argument));;

	for (x = 0; swear_table[x].level > 0; x++) {
		switch (swear_table[x].level) {
		case 1:
			if (is_exact_name(swear_table[x].word, tobechecked))
				return TRUE;

		case 2:
			if (is_name(swear_table[x].word, tobechecked))
				return TRUE;
		}
	}

	return FALSE;
} /* end swearcheck() */

bool check_channel_social(CHAR_DATA *ch, int channel, int custom, const String& command, const String& argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	struct social_type *iterator;
	bool found;
	found  = FALSE;

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		if (command[0] == iterator->name[0]
		    &&   !str_prefix1(command, iterator->name)) {
			found = TRUE;
			break;
		}
	}

	if (!found)
		return FALSE;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS)) {
		stc("You are anti-social!\n", ch);
		return TRUE;
	}

	String arg;
	one_argument(argument, arg);

	victim = get_player_world(ch, arg, VIS_PLR);
	new_color(ch, custom);

	if (arg.empty()) {
		if (iterator->char_no_arg != NULL)
			stc("[S] ", ch);

		act(iterator->char_no_arg,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}
	else if (victim == NULL) {
		stc("[S] They are not here.\n", ch);
		return TRUE;
	}
	else if (victim == ch) {
		if (iterator->char_auto != NULL)
			stc("[S] ", ch);

		act(iterator->char_auto,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}
	else {
		if (iterator->char_found != NULL)
			stc("[S] ", ch);

		act(iterator->char_found,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}

	set_color(ch, WHITE, NOBOLD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *vic;
		vic = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    !is_ignoring(vic, ch) &&
		    !IS_SET(vic->comm, channel) &&
		    !IS_SET(vic->comm, COMM_QUIET)) {
			new_color(vic, custom);

			if (arg.empty()) {
				if (iterator->others_no_arg != NULL)
					stc("[S] ", vic);

				act(iterator->others_no_arg, ch, NULL, vic, TO_VICT, POS_SLEEPING, FALSE);
			}
			else if (victim == ch) {
				if (iterator->others_auto != NULL)
					stc("[S] ", vic);

				act(iterator->others_auto, ch, vic, victim, TO_WORLD, POS_SLEEPING, FALSE);
			}
			else if (vic == victim) {
				if (iterator->vict_found != NULL)
					stc("[S] ", vic);

				act(iterator->vict_found,  ch, NULL, victim, TO_VICT, POS_SLEEPING, FALSE);
			}
			else {
				if (iterator->others_found != NULL)
					stc("[S] ", vic);

				act(iterator->others_found, ch, vic, victim, TO_WORLD, POS_SLEEPING, FALSE);
			}

			set_color(vic, WHITE, NOBOLD);
		}
	}

	return TRUE;
}

/* Channel who by Lotus */
void channel_who(CHAR_DATA *ch, const char *channelname, int channel, int custom)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch))
		return;

	new_color(ch, custom);
	ptc(ch, "Players with %s ON\n", channelname);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if ((channel == COMM_NOWIZ && !IS_IMMORTAL(d->character)) ||
		    (channel == COMM_NOCLAN && !is_same_clan(ch, victim)))
			continue;

		if (IS_PLAYING(d) &&
		    can_see_who(ch, victim) &&
		    !IS_SET(victim->comm, channel) &&
		    !IS_SET(victim->comm, COMM_NOCHANNELS) &&
		    !IS_SET(victim->comm, COMM_QUIET)) {
			if (channel == COMM_NOFLAME && IS_SET(victim->censor, CENSOR_CHAN))
				continue;

			ptc(ch, "%s\n", victim->name);
		}
	}

	return;
}

/* This routine is for query call statements, checks to see which users in
the query list are on, and then sends the mesg to them */
void send_to_query(CHAR_DATA *ch, const char *string)
{
	// really hate directly accessing the pc_list, but I don't want multiple
	// calls to get_player_world.
	for (PC_DATA *pc = pc_list; pc; pc = pc->next) {
		if (!pc->ch
		    || IS_NPC(pc->ch)
		    || IS_SET(pc->ch->comm, COMM_NOQUERY)
		    || is_ignoring(pc->ch, ch))
			continue;

		if (std::find(ch->pcdata->query.cbegin(), ch->pcdata->query.cend(), pc->ch->name) != ch->pcdata->query.cend()) {
			new_color(pc->ch, CSLOT_CHAN_QTELL);
			stc(string, pc->ch);
			set_color(pc->ch, WHITE, NOBOLD);
		}
	}
}

void send_to_clan(CHAR_DATA *ch, CLAN_DATA *target, const char *text)
{
	DESCRIPTOR_DATA *d;

	if (target == NULL) {
		stc("No such clan!\n", ch);
		return;
	}

	if (text[0] == '\0') {
		stc("Tell them what ?\n", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (IS_PLAYING(d) &&
		    d->character->clan == target)
			stc(text, d->character);
	}
}

void wiznet(const String& string, CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip, int min_rank)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (IS_PLAYING(d)
		    && IS_IMMORTAL(d->character)
		    && IS_SET(d->character->wiznet, WIZ_ON)
		    && (!flag || IS_SET(d->character->wiznet, flag))
		    && (!flag_skip || IS_SET(d->character->wiznet, flag_skip))
		    && GET_RANK(d->character) >= min_rank
		    && d->character != ch) {
			if (IS_SET(d->character->wiznet, WIZ_PREFIX))
				stc("{G<W{HizNe{Gt>{x ", d->character);

			act(string, d->character, obj, ch, TO_CHAR, POS_DEAD, FALSE);
		}
	}
}

void channel(CHAR_DATA *ch, const String& argument, int channel)
{
	DESCRIPTOR_DATA *d;
	int cslot = chan_table[channel].cslot;

	if (channel == CHAN_CLAN) {
		if (!is_clan(ch) || (ch->clan->independent && !IS_IMMORTAL(ch))) {
			stc("You aren't in a clan.\n", ch);
			return;
		}
	}

	if (argument.empty()) {
		new_color(ch, cslot);

		if (IS_SET(ch->comm, chan_table[channel].bit)) {
			ptc(ch, "The %s channel is now ON.\n", chan_table[channel].name);
			REMOVE_BIT(ch->comm, chan_table[channel].bit);
		}
		else {
			ptc(ch, "The %s channel is now OFF.\n", chan_table[channel].name);
			SET_BIT(ch->comm, chan_table[channel].bit);
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET) && channel != CHAN_IMMTALK) {
		new_color(ch, cslot);
		stc("You must turn off QUIET mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY) && channel != CHAN_FLAME) {
		stc("The gods have restricted you to the flame channel.\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, chan_table[channel].revoke_bit)) {
		ptc(ch, "The gods have revoked your ability to use %s.\n", chan_table[channel].name);
		return;
	}

	if (get_position(ch) < POS_SLEEPING) {
		stc("You are hurt too bad for that.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!str_cmp(arg, "who") && argument[3] == '\0') {
		channel_who(ch, chan_table[channel].name, chan_table[channel].bit, cslot);
		return;
	}

	REMOVE_BIT(ch->comm, chan_table[channel].bit);
	new_color(ch, cslot);

	if (channel == CHAN_IMMTALK) { /* lil different for immtalk */
		char prefix[MSL];

		if (ch->pcdata && ch->pcdata->immprefix[0] != '\0')
			Format::sprintf(prefix, "%s", ch->pcdata->immprefix);
		else
			Format::sprintf(prefix, "%s:", IS_NPC(ch) ? ch->short_descr : ch->name);

		ptc(ch, "[%s",
		    ch->secure_level == RANK_IMP ? "{YIMP" :
		    ch->secure_level == RANK_HEAD ? "{BHEAD" :
		    "IMM");
		new_color(ch, cslot);
		ptc(ch, "] %s", prefix);
		new_color(ch, cslot);
		ptc(ch, " %s{x\n", argument);
	}
	else
		ptc(ch, "%s {x'%s{x'\n", chan_table[channel].prefix_self, argument);

	set_color(ch, WHITE, NOBOLD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) && d->character != ch) {
			if (channel == CHAN_IMMTALK) { /* we can skip a lot of the below junk */
				if (!IS_IMMORTAL(victim)
				    || GET_RANK(victim) < ch->secure_level
				    || IS_SET(victim->comm, chan_table[channel].bit))
					continue;

				char prefix[MSL];
				if (can_see_who(victim, ch)) {
					if (ch->pcdata && ch->pcdata->immprefix[0] != '\0')
						Format::sprintf(prefix, "%s", ch->pcdata->immprefix);
					else
						Format::sprintf(prefix, "%s:", IS_NPC(ch) ? ch->short_descr : ch->name);
				}
				else
					Format::sprintf(prefix, "Someone:");

				new_color(victim, cslot);
				ptc(victim, "[%s",
				    ch->secure_level == RANK_IMP ? "{YIMP" :
				    ch->secure_level == RANK_HEAD ? "{BHEAD" :
				    "IMM");
				new_color(victim, cslot);
				ptc(victim, "] %s", prefix);
				new_color(victim, cslot);
				ptc(victim, " %s{x\n", argument);
				set_color(victim, WHITE, NOBOLD);
				continue;
			}

			if (is_ignoring(victim, ch)
			    || IS_SET(victim->comm, chan_table[channel].bit)
			    || IS_SET(victim->comm, COMM_QUIET))
				continue;

			if (channel == CHAN_PRAY && !IS_IMMORTAL(victim))
				continue;

			if (channel == CHAN_FLAME && IS_SET(victim->censor, CENSOR_CHAN))
				continue;

			if (channel != CHAN_FLAME && swearcheck(argument)) {
				if (IS_SET(victim->censor, CENSOR_CHAN)
				    && channel != CHAN_PRAY) /* don't censor pray */
					continue;

				if (IS_IMMORTAL(victim))
					stc("{P[C] {x", victim);
			}

			if (channel == CHAN_CLAN) {
				if (!is_same_clan(ch, victim) && !IS_IMMORTAL(victim))
					continue;

				if (IS_IMMORTAL(victim))
					ptc(victim, "%s ", ch->clan->who_name);
			}

			new_color(victim, cslot);

			/* special for pray, reveal their original character */
			if (channel == CHAN_PRAY && ch->desc && ch->desc->original)
				ptc(victim, "%s (%s) implores the gods: \"%s{x\"\n",
				    ch->desc->original->name, ch->name, argument);
			else
				/* -----> */                    act(chan_table[channel].other_str,
				                                        ch, argument, victim, TO_VICT_CHANNEL, POS_SLEEPING, FALSE);

			set_color(victim, WHITE, NOBOLD);
		}
	}

}

void do_gossip(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_GOSSIP);
}

void do_flame(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_FLAME);
}

void do_qwest(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_QWEST);
}

void do_pray(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_PRAY);
}

void do_clantalk(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_CLAN);
}

void do_music(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_MUSIC);
}

void do_ic(CHAR_DATA *ch, String argument)
{
	if (IS_NPC(ch)) {
		stc("Just be yourself, no need to pretend :)\n", ch);
		return;
	}

	if (!IS_SET(ch->pcdata->plr, PLR_OOC)) {
		stc("You are not in character (help ooc).\n", ch);
		return;
	}

	channel(ch, argument, CHAN_IC);
}

void do_grats(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_GRATS);
}

void do_immtalk(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_IMMTALK);
}

void do_question(CHAR_DATA *ch, String argument)
{
	channel(ch, argument, CHAN_QA);
}

void talk_auction(const char *argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d)
		    && IS_SET(victim->censor, CENSOR_CHAN))
			if (swearcheck(argument))
				continue;

		if (IS_PLAYING(d)
		    && !IS_SET(victim->comm, COMM_NOAUCTION)
		    && !IS_SET(victim->comm, COMM_QUIET)) {
			new_color(victim, CSLOT_CHAN_AUCTION);
			ptc(victim, "AUCTION: %s", argument);
			set_color(victim, WHITE, NOBOLD);
		}
	}
}

void do_announce(CHAR_DATA *ch, String argument)
{
	if (IS_SET(ch->comm, COMM_NOANNOUNCE)) {
		new_color(ch, CSLOT_CHAN_ANNOUNCE);
		stc("Announcements will now be shown.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		REMOVE_BIT(ch->comm, COMM_NOANNOUNCE);
	}
	else {
		new_color(ch, CSLOT_CHAN_ANNOUNCE);
		stc("Announce channel is now OFF.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		SET_BIT(ch->comm, COMM_NOANNOUNCE);
	}
}

void do_send_announce(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d)
		    && d->character != ch
		    && !IS_SET(victim->comm, COMM_NOANNOUNCE)
		    && !IS_SET(victim->comm, COMM_QUIET)) {
			new_color(victim, CSLOT_CHAN_ANNOUNCE);
			ptc(victim, "[FYI] %s{x\n", argument);
			set_color(victim, WHITE, NOBOLD);
		}
	}

}

/* Lotus - Let us Imms use the FYI Channel for jokes */
void do_fyi(CHAR_DATA *ch, String argument)
{
	DESCRIPTOR_DATA *d;
	new_color(ch, CSLOT_CHAN_ANNOUNCE);
	ptc(ch, "You FYI '%s{x'\n", argument);
	set_color(ch, WHITE, NOBOLD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    IS_SET(victim->censor, CENSOR_CHAN))
			if (swearcheck(argument))
				continue;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    !is_ignoring(victim, ch) &&
		    !IS_SET(victim->comm, COMM_NOANNOUNCE) &&
		    !IS_SET(victim->comm, COMM_QUIET)) {
			new_color(victim, CSLOT_CHAN_ANNOUNCE);

			if (IS_IMMORTAL(victim))
				act("[$n{x] $t{x",
				        ch, argument, d->character, TO_VICT, POS_SLEEPING, FALSE);
			else
				act("[FYI] $t{x",
				        ch, argument, d->character, TO_VICT, POS_SLEEPING, FALSE);

			set_color(ch, WHITE, NOBOLD);
		}
	}

}

void do_replay(CHAR_DATA *ch, String argument)
{
	if (IS_NPC(ch)) {
		stc("{YMobiles can't work answering machines.{x\n", ch);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	page_to_char(buf_string(ch->pcdata->buffer), ch);
	set_color(ch, WHITE, NOBOLD);
	clear_buf(ch->pcdata->buffer);
}

/* Channel specifically for socials and emotes by Lotus */
void do_globalsocial(CHAR_DATA *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	if (argument.empty()) {
		if (IS_SET(ch->comm, COMM_NOSOCIAL)) {
			new_color(ch, CSLOT_CHAN_SOCIAL);
			stc("Social channel is now ON.\n", ch);
			REMOVE_BIT(ch->comm, COMM_NOSOCIAL);
			set_color(ch, WHITE, NOBOLD);
		}
		else {
			new_color(ch, CSLOT_CHAN_SOCIAL);
			stc("Social channel is now OFF.\n", ch);
			SET_BIT(ch->comm, COMM_NOSOCIAL);
			set_color(ch, WHITE, NOBOLD);
		}

		return;
	}

	/* social sent, turn Social on if it isn't already */
	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_SOCIAL);
		stc("You must turn off quiet mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY)) {
		stc("The gods have restricted you to the flame channel.\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_SOCIAL)) {
		stc("The gods have revoked your ability to use this channel.\n", ch);
		return;
	}

	String arg;
	String arg2 = one_argument(argument, arg);

	if (!str_prefix1(arg, "who") && argument[3] == '\0') {
		channel_who(ch, "Social", COMM_NOSOCIAL, CSLOT_CHAN_SOCIAL);
		return;
	}

	REMOVE_BIT(ch->comm, COMM_NOSOCIAL);

	/* channel social: SOCIAL <social> */
	/* This checks for predefined socials. -- Outsider */
	if (check_channel_social(ch, COMM_NOSOCIAL, CSLOT_CHAN_SOCIAL, arg, arg2))
		return;

	/***** social pose removed -- Elrac
	if (!str_prefix1( arg, "pose" ))
	{
	    pose = select_pose( ch );
	    if ( pose < 0 ) return;
	    Format::sprintf( buf, "[P] %s\n",
	        new_pose_table[ch->cls].poses[pose].self_msg );
	}
	else
	*****/
	if (!str_prefix1(arg, "emote") && (!arg2.empty()))
		Format::sprintf(buf, "[E] %s %s\n", ch->name, arg2);
	else
		Format::sprintf(buf, "You socialize '%s{x'\n", argument);

	/* send social to player himself */
	new_color(ch, CSLOT_CHAN_SOCIAL);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	/* set up message to send everybody */

	/* no channel poses */
	/*    if ( pose != -1 )
	        Format::sprintf( buf, "[P] %s\n",
	            new_pose_table[ch->cls].poses[pose].room_msg );
	*/

	if (!str_prefix1(arg, "emote") && (!arg2.empty()))
		Format::sprintf(buf, "[E] $n %s", arg2);
	else
		Format::sprintf(buf, "$n socializes '%s{x'", argument);

	/* broadcast the social */
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    IS_SET(victim->censor, CENSOR_CHAN))
			if (swearcheck(argument))
				continue;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    !is_ignoring(victim, ch) &&
		    !IS_SET(victim->comm, COMM_NOSOCIAL) &&
		    !IS_SET(victim->comm, COMM_QUIET)) {
			new_color(victim, CSLOT_CHAN_SOCIAL);
			act(buf, ch, NULL, victim, TO_VICT, POS_SLEEPING, FALSE);
			set_color(victim, WHITE, NOBOLD);
		}
	}   /* end of for loop -- to each player */

}

void do_iclantalk(CHAR_DATA *ch, String argument)
{
	CLAN_DATA *clan, *oclan;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		new_color(ch, CSLOT_CHAN_CLAN);
		stc("Syntax: iclan <clan name> <message>\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((clan = clan_lookup(arg)) == NULL) {
		stc("No such clan exists, type clanlist for a list of valid clans.\n", ch);
		return;
	}

	oclan = ch->clan;
	ch->clan = clan;
	ptc(ch, "%s ", clan->who_name);
	do_clantalk(ch, argument);
	ch->clan = oclan;
}

/* Improved do_say (With Colour!) - Xenith */
void do_say(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *vch;

	if (ch->in_room == NULL)
		return;

	if (argument.empty()) {
		stc("Say what?\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_SAY)) {
		stc("You are unable to talk!\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_SILENT)) {
		stc("A heavy mist dampens all sound in the room.\n", ch);
		return;
	}

	argument = makedrunk(ch, argument);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch != ch && IS_AWAKE(vch)) {
			new_color(vch, CSLOT_CHAN_SAY);

			/* make say command handle yelling and questions */
			if (argument[ strlen(argument) - 1] == '?')
				ptc(vch, "%s asks '%s'\n", PERS(ch, vch, VIS_CHAR), argument);
			else if (argument[ strlen(argument) - 1] == '!')
				ptc(vch, "%s exclaims '%s'\n", PERS(ch, vch, VIS_CHAR), argument);
			else
				ptc(vch, "%s says '%s'\n", PERS(ch, vch, VIS_CHAR), argument);

			set_color(vch, WHITE, NOBOLD);
		}
	}

	/* viewing room stuff */
	/* make say command handle yelling and questions */
	if (argument[ strlen(argument) - 1] == '?')
		act("$n asks '$T'\n", ch, NULL, argument, TO_VIEW);
	else if (argument[ strlen(argument) - 1] == '!')
		act("$n exclaims '$T'\n", ch, NULL, argument, TO_VIEW);
	else
		act("$n says '$T'\n", ch, NULL, argument, TO_VIEW);

	new_color(ch, CSLOT_CHAN_SAY);

	/* one more time...make say command handle questions and yelling */
	if (argument[ strlen(argument) - 1] == '?')
		ptc(ch, "You ask '%s'\n", argument);
	else if (argument[ strlen(argument) - 1] == '!')
		ptc(ch, "You exclaim '%s'\n", argument);
	else
		ptc(ch, "You say '%s'\n", argument);

	set_color(ch, WHITE, NOBOLD);
	mprog_speech_trigger(argument, ch);
}

void do_tell(CHAR_DATA *ch, String argument)
{
	char buf[MSL];
	CHAR_DATA *victim;
	char *strtime;

	if (IS_SET(ch->revoke, REVOKE_TELL)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("The Imms have rescinded your ability to TELL.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("You must turn off quiet mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_DEAF)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("You must turn off deaf mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Tell whom what?\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* Can tell to PC's anywhere, but NPC's only in same room. */
	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
		if ((victim = get_mob_here(ch, arg, VIS_CHAR)) == NULL) {
			new_color(ch, CSLOT_CHAN_TELL);
			stc("They aren't here.\n", ch);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

	if (IS_SET(victim->comm, COMM_QUIET | COMM_DEAF | COMM_NOCHANNELS) && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		act("$E is not receiving tells.", ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		ptc(ch, "%s is ignoring you.\n", victim->name);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (victim->desc == NULL && !IS_NPC(victim)) {
		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act("$E has lost $S link, but your message will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		Format::sprintf(buf, "[%s] %s tells you '%s{x'\n", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	if (IS_SET(victim->comm, COMM_AFK)) {
		if (IS_NPC(victim)) {
			new_color(ch, CSLOT_CHAN_TELL);
			act("$E is AFK, and not receiving tells.",
			        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act("$E is AFK, but your tell will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		stc(victim->pcdata->afk, ch);
		set_color(ch, WHITE, NOBOLD);
		Format::sprintf(buf, "[%s] %s tells you '%s{x'\n", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	act("You tell $N '$t{x'", ch, argument, victim, TO_CHAR, POS_SLEEPING, FALSE);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_TELL);
	act("$n tells you '$t{x'", ch, argument, victim, TO_VICT, POS_SLEEPING, FALSE);
	set_color(victim, WHITE, NOBOLD);

	if (IS_NPC(victim) || !victim->replylock)
		strcpy(victim->reply, ch->name);
}

void do_reply(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char *strtime;
	bool found = FALSE;

	if (argument.empty()) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Reply with what?\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_TELL)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Your message didn't get through.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (ch->reply[0] == '\0') {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("But no one has sent you a tell yet!\n", ch);
		set_color(ch, WHITE, NOBOLD);
		ch->replylock = FALSE;
		return;
	}

	for (victim = char_list; victim != NULL ; victim = victim->next)
		if (! strcmp(ch->reply, victim->name)) {
			found = TRUE;
			break;
		}

	if (!found) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("They aren't here.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		ch->replylock = FALSE;
		return;
	}

	/* Lock replies to someone, idea by Scattercat */
	if (!str_cmp(argument, "lock") && !IS_NPC(ch)) {
		ch->replylock = !ch->replylock;

		if (!ch->replylock)
			ptc(ch, "You unlock your replies from %s.\n", PERS(victim, ch, VIS_PLR));
		else
			ptc(ch, "You lock your replies to %s.\n", PERS(victim, ch, VIS_PLR));

		return;
	}

	if ((IS_SET(victim->comm, COMM_QUIET)
	     || IS_SET(victim->comm, COMM_DEAF)
	     || IS_SET(victim->comm, COMM_NOCHANNELS))
	    && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		act("$E is not receiving tells.", ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		ptc(ch, "%s is ignoring you.\n", victim->name);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (victim->desc == NULL && !IS_NPC(victim)) {
		strtime = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act("$N seems to have misplaced $S link...try again later.", ch, NULL, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		Format::sprintf(buf, "[%s] %s tells you '%s{x'\n", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	if (IS_SET(victim->comm, COMM_AFK)) {
		if (IS_NPC(victim)) {
			new_color(ch, CSLOT_CHAN_TELL);
			act("$E is AFK, and not receiving tells.",
			        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

		strtime = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act("$E is AFK, but your tell will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		act(victim->pcdata->afk, ch, NULL, NULL, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		Format::sprintf(buf, "[%s] %s tells you '%s{x'\n", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	act("You tell $N '$t{x'", ch, argument, victim, TO_CHAR, POS_DEAD, FALSE);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_TELL);
	act("$n tells you '$t{x'", ch, argument, victim, TO_VICT, POS_DEAD, FALSE);
	set_color(victim, WHITE, NOBOLD);

	if (IS_NPC(victim))
		strcpy(victim->reply, ch->name);
	else if (!victim->replylock)
		strcpy(victim->reply, ch->name);
}

void do_yell(CHAR_DATA *ch, String argument)
{
	DESCRIPTOR_DATA *d;

	if (argument.empty()) {
		stc("You yell your head off but no one hears.\n", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n", ch);
		return;
	}

	act("You yell '$t{x'", ch, argument, NULL, TO_CHAR);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    d->character != ch &&
		    IS_SET(victim->censor, CENSOR_CHAN))
			if (swearcheck(argument))
				continue;

		if (IS_PLAYING(d)
		    &&   d->character != ch
		    &&   d->character->in_room != NULL
		    &&   d->character->in_room->area == ch->in_room->area
		    &&   !is_ignoring(victim, ch)
		    &&   !IS_SET(d->character->comm, COMM_QUIET))
			act("$n yells '$t{x'", ch, argument, d->character, TO_VICT);
	}

	return;
}

void do_emote(CHAR_DATA *ch, String argument)
{
	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_EMOTE)) {
		stc("You're not feeling very emotional right now.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Emote what?\n", ch);
		return;
	}

	act("$n $T{x", ch, NULL, argument, TO_ROOM);
	act("$n $T{x", ch, NULL, argument, TO_CHAR);
	mprog_speech_trigger(argument, ch);
	return;
}

void do_pmote(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *vch;
	const char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	unsigned int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_EMOTE)) {
		stc("You're not feeling very emotional right now.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Emote what?\n", ch);
		return;
	}

	act("$n $t{x", ch, argument, NULL, TO_CHAR);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			act("$N $t{x", vch, argument, ch, TO_CHAR);
			continue;
		}

		strcpy(temp, argument);
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name.c_str();

		for (; *letter != '\0'; letter++) {
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strcat(temp, "r");
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name)) {
				matches = 0;
				continue;
			}

			if (matches == strlen(vch->name))
				matches = 0;

			if (*letter == *name) {
				matches++;
				name++;

				if (matches == strlen(vch->name)) {
					strcat(temp, "you");
					last[0] = '\0';
					name = vch->name.c_str();
					continue;
				}

				strncat(last, letter, 1);
				continue;
			}

			matches = 0;
			strcat(temp, last);
			strncat(temp, letter, 1);
			last[0] = '\0';
			name = vch->name.c_str();
		}

		act("$N $t{x", vch, temp, ch, TO_CHAR);
	}

	return;
}

void do_smote(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *vch;
	const char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	unsigned int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS)) {
		stc("You cannot show your emotions.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("S-Emote what?\n", ch);
		return;
	}

	if (strstr(argument, ch->name) == NULL) {
		stc("You must include your name in an s-emote.\n", ch);
		return;
	}

	stc(argument, ch);
	stc("\n", ch);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			stc(argument, vch);
			stc("\n", vch);
			continue;
		}

		strcpy(temp, argument);
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name.c_str();

		for (; *letter != '\0'; letter++) {
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strcat(temp, "r");
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name)) {
				matches = 0;
				continue;
			}

			if (matches == strlen(vch->name))
				matches = 0;

			if (*letter == *name) {
				matches++;
				name++;

				if (matches == strlen(vch->name)) {
					strcat(temp, "you");
					last[0] = '\0';
					name = vch->name.c_str();
					continue;
				}

				strncat(last, letter, 1);
				continue;
			}

			matches = 0;
			strcat(temp, last);
			strncat(temp, letter, 1);
			last[0] = '\0';
			name = vch->name.c_str();
		}

		stc(temp, vch);
		stc("\n", vch);
	}

	return;
}

void do_page(CHAR_DATA *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	char *strtime;

	if (IS_SET(ch->revoke, REVOKE_PAGE)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("The Imms have rescinded your ability to PAGE.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("You must turn off quiet mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_DEAF)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("You must turn off deaf mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		new_color(ch, CSLOT_CHAN_PAGE);

		if (IS_SET(ch->comm, COMM_NOPAGE)) {
			stc("You turn on your pager.\n", ch);
			REMOVE_BIT(ch->comm, COMM_NOPAGE);
		}
		else {
			stc("You turn off your pager.\n", ch);
			SET_BIT(ch->comm, COMM_NOPAGE);
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		new_color(ch, CSLOT_CHAN_PAGE);
		ptc(ch, "You failed to locate [%s] in these realms.\n", arg);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((IS_SET(victim->comm, COMM_NOPAGE))
	    && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		act("$E does not have $S pager turned on.", ch, NULL, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		ptc(ch, "%s is ignoring you.\n", victim->name);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (victim->desc == NULL) {
		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		new_color(ch, CSLOT_CHAN_PAGE);
		act("$N seems to have misplaced $S link...try again later.",
		    ch, NULL, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		Format::sprintf(buf, "{R[%s] %s PAGES '%s{x'{x\a\n",
		        strtime , PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);

		if (!IS_NPC(victim)) add_buf(victim->pcdata->buffer, buf);

		return;
	}

	new_color(ch, CSLOT_CHAN_PAGE);
	act("You PAGE $N '$t{x'", ch, argument, victim, TO_CHAR, POS_SLEEPING, FALSE);
	stc("Thank you for using Comcast Telecommunications, Inc.\n", ch);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_PAGE);
	act("$n PAGES '$t{x'\a", ch, argument, victim, TO_VICT, POS_SLEEPING, FALSE);
	set_color(victim, WHITE, NOBOLD);
	strcpy(victim->reply, ch->name);
	return;
}

void do_whisper(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *victim;

	if (IS_NPC(ch)) {
		stc("You lack the refined delicacy of voice to whisper.\n", ch);
		return;
	}

	if (ch->pcdata->whisper[0]) {
		do_huh(ch);
		return;
	}

	if ((victim = get_player_world(ch, ch->pcdata->whisper, VIS_PLR)) == NULL) {
		ptc(ch, "{G%s isn't here. :({x\n", ch->pcdata->whisper);
		return;
	}

	if (argument.empty()) {
		ptc(victim, "{G%s whispers sweet nothings into your ear.{x\n", ch->name);
		ptc(ch, "{GYou whisper sweet nothings into %s's ear.{x\n", victim->name);
		return;
	}

	ptc(victim, "{G%s whispers in your ear, '%s{G'{x\n", ch->name, argument);
	ptc(ch, "{GYou whisper in %s's ear, '%s{G'{x\n", victim->name, argument);
}

void do_qtell(CHAR_DATA *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		stc("You do not have a query list.\n", ch);
		return;
	}

	if (argument.empty()) {
		if (IS_SET(ch->comm, COMM_NOQUERY)) {
			new_color(ch, CSLOT_CHAN_QTELL);
			stc("Query channel is now on.\n", ch);
			REMOVE_BIT(ch->comm, COMM_NOQUERY);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
		else {
			new_color(ch, CSLOT_CHAN_QTELL);
			stc("Query channel is now off.\n", ch);
			SET_BIT(ch->comm, COMM_NOQUERY);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
	}

	if (!ch->pcdata->query[0][0]) {
		stc("You have no one on your query list.\n", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_QTELL)) {
		stc("Your message didn't get through!\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOQUERY)) {
		new_color(ch, CSLOT_CHAN_QTELL);
		stc("Query channel is now on.\n", ch);
		REMOVE_BIT(ch->comm, COMM_NOQUERY);
		set_color(ch, WHITE, NOBOLD);
	}

	new_color(ch, CSLOT_CHAN_QTELL);
	ptc(ch, "You tell the query '%s{x'\n", argument);
	set_color(ch, WHITE, NOBOLD);
	Format::sprintf(buf, "%s tells the query '%s{x'\n", ch->name, argument);
	send_to_query(ch, buf);
	return;
}

void do_gtell(CHAR_DATA *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;

	if (argument.empty()) {
		new_color(ch, CSLOT_CHAN_GTELL);
		stc("Tell your group what?\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/*
	 * Note use of stc, so gtell works on sleepers.
	 */
	Format::sprintf(buf, "%s tells the group '%s{x'\n", ch->name, argument);

	/* would be more efficient to find leader and step thru all his group members
	-- Elrac */
	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (is_same_group(gch, ch)) {
			new_color(gch, CSLOT_CHAN_GTELL);
			stc(buf, gch);
			set_color(gch, WHITE, NOBOLD);
		}
	}

	return;
}

void do_query(CHAR_DATA *ch, String argument)
{
	CHAR_DATA *rch;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	String arg, arg2;
	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if (str_cmp(arg, "+") && str_cmp(arg, "-")
	    && str_prefix1(arg, "list") && str_prefix1(arg, "clear")) {
		stc("Syntax:\n", ch);
		stc("{RQuery list{x       : Lists names in your query.\n", ch);
		stc("{RQuery +{x <{YName{x>   : Adds a name to your query.\n", ch);
		stc("{RQuery -{x <{YName{x>   : Removes a name from query.\n", ch);
		stc("{RQuery clear{x      : Clears your query (removes all names).\n", ch);
		stc(" ", ch);
		stc("{RQtell{x            : Turns your query channel on or off.\n", ch);
		return;
	}

	if (!str_prefix1(arg, "list")) {
		if (rch->pcdata->query.empty()) {
			stc("You have no one in your query.\n", ch);
			return;
		}

		stc("People in your query:\n", ch);

		for (auto it = rch->pcdata->query.cbegin(); it != rch->pcdata->query.end(); it++)
			ptc(ch, " %s\n", (*it).c_str());

		return;
	}

	if (!str_prefix1(arg, "clear")) {
		rch->pcdata->query.clear();
		stc("Query cleared.\n", ch);
		return;
	}

	if (!str_prefix1(arg, "+")) {
		if (arg2.empty()) {
			stc("Add who to the query list?\n", ch);
			return;
		}

		if (std::find(rch->pcdata->query.cbegin(), rch->pcdata->query.cend(), arg2) != rch->pcdata->query.cend()) {
			stc("That name is already in the query.\n", ch);
			return;
		}

		if (rch->pcdata->query.size() >= MAX_QUERY) {
			stc("Too many users in query, remove a name.\n", ch);
			return;
		}

		rch->pcdata->query.push_back(arg2);
		ptc(ch, "%s has been added to the query list.\n", arg2);
		return;
	}

	if (!str_prefix1(arg, "-")) {
		if (arg2.empty()) {
			stc("Remove who from the query list?\n", ch);
			return;
		}

		auto search = std::find(rch->pcdata->query.begin(), rch->pcdata->query.end(), arg2);

		if (search != rch->pcdata->query.end()) {
			stc("Name removed from the query.\n", ch);
			rch->pcdata->query.erase(search);
		}
		else
			stc("That name was not found in the query list.\n", ch);
	}
}

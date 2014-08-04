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

#include <sys/types.h>
//#include<time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"

#include "chit.h"


extern void     channel_who     args((CHAR_DATA *ch, char *channelname, int
                                      channel, int custom));
bool    check_channel_social    args((CHAR_DATA *ch, int channel,
                                      int custom, char *command, char *argument));

void    send_to_query           args((CHAR_DATA *ch, char *string));
bool    swearcheck              args((char *argument));
extern bool     is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim);
char   *makedrunk               args((CHAR_DATA *ch, char *string));


/* RT code to display channel status */
void do_channels(CHAR_DATA *ch, char *argument)
{
	/* lists all channels and their status */
	stc("   channel     status\n\r", ch);
	stc("----------------------\n\r", ch);
	new_color(ch, CSLOT_CHAN_GOSSIP);
	stc("gossip         ", ch);

	if (IS_SET(ch->revoke, REVOKE_GOSSIP)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOGOSSIP)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_FLAME);
	stc("flame          ", ch);

	if (IS_SET(ch->revoke, REVOKE_FLAME)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOFLAME)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_QWEST);
	stc("qwest          ", ch);

	if (IS_SET(ch->revoke, REVOKE_QWEST)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOQWEST)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_PRAY);
	stc("pray           ", ch);

	if (IS_SET(ch->revoke, REVOKE_PRAY)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOPRAY)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_AUCTION);
	stc("auction        ", ch);

	if (IS_SET(ch->revoke, REVOKE_AUCTION)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOAUCTION)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	if (is_clan(ch) && (!ch->clan->independent || IS_IMMORTAL(ch))) {
		new_color(ch, CSLOT_CHAN_CLAN);
		stc("Clantalk       ", ch);

		if (IS_SET(ch->revoke, REVOKE_CLAN)) stc("REVOKED\n\r", ch);
		else if (!IS_SET(ch->comm, COMM_NOCLAN)) stc("ON\n\r", ch);
		else stc("OFF\n\r", ch);
	}

	new_color(ch, CSLOT_CHAN_ANNOUNCE);
	stc("announce       ", ch);

	if (!IS_SET(ch->comm, COMM_NOANNOUNCE)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_MUSIC);
	stc("music          ", ch);

	if (IS_SET(ch->revoke, REVOKE_MUSIC)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOMUSIC)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_QA);
	stc("Q/A            ", ch);

	if (IS_SET(ch->revoke, REVOKE_QA)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOQUESTION)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_SOCIAL);
	stc("Social         ", ch);

	if (IS_SET(ch->revoke, REVOKE_SOCIAL)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOSOCIAL)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_IC);
	stc("IC             ", ch);

	if (IS_SET(ch->revoke, REVOKE_IC)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOIC)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_GRATS);
	stc("grats          ", ch);

	if (IS_SET(ch->revoke, REVOKE_GRATS)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOGRATS)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	if (IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_IMM);
		stc("Imm Chat       ", ch);

		if (!IS_SET(ch->comm, COMM_NOWIZ)) stc("ON\n\r", ch);
		else stc("OFF\n\r", ch);
	}

	new_color(ch, CSLOT_CHAN_PAGE);
	stc("page           ", ch);

	if (IS_SET(ch->revoke, REVOKE_PAGE)) stc("REVOKED\n\r", ch);
	else if (!IS_SET(ch->comm, COMM_NOPAGE)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	new_color(ch, CSLOT_CHAN_TELL);
	stc("tell           ", ch);

	if (!IS_SET(ch->comm, COMM_DEAF)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	/* Check if we receive remote channels. -- Outsider */
	set_color(ch, WHITE, NOBOLD);
	stc("remote         ", ch);

	if (ch->pcdata->block_remote) stc("OFF\n\r", ch);
	else stc("ON\n\r", ch);

	set_color(ch, WHITE, NOBOLD);
	new_color(ch, CSLOT_CHAN_QTELL);
	stc("qtell          ", ch);

	if (IS_SET(ch->revoke, REVOKE_QTELL)) stc("REVOKED\n\n", ch);
	else if (!IS_SET(ch->comm, COMM_NOQUERY)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	set_color(ch, WHITE, NOBOLD);
	stc("quiet mode     ", ch);

	if (IS_SET(ch->comm, COMM_QUIET)) stc("ON\n\r", ch);
	else stc("OFF\n\r", ch);

	/*
	 * Stop the mud from crashing when morphed players use channels
	*/
	if (!IS_NPC(ch)) {
		if (IS_SET(ch->pcdata->plr, PLR_NONOTIFY)) stc("You will not be notified of new notes.\n\r", ch);
		else stc("You {Wwill{x be notified of new notes.\n\r", ch);

		if (ch->pcdata->aura && ch->pcdata->aura[0] != '\0')
			ptc(ch, "{VAura: (%s{V){x\n\r", ch->pcdata->aura);
	}

	if (IS_SET(ch->comm, COMM_AFK)) stc("You are AFK.\n\r", ch);

	if (!IS_NPC(ch)) if (IS_SET(ch->pcdata->plr, PLR_SNOOP_PROOF)) stc("You are immune to Nosy people.\n\r", ch);

	if (ch->lines != PAGELEN) {
		if (ch->lines)
			ptc(ch, "You display %d lines of scroll.\n\r", ch->lines);
		else
			stc("Scroll buffering is off.\n\r", ch);
	}

	if (ch->prompt != NULL)
		ptc(ch, "Your current prompt is: %s\n\r", ch->prompt);

	if (IS_SET(ch->revoke, REVOKE_TELL))
		stc("No one wants to listen to you.\n\r", ch);

	if (IS_SET(ch->revoke, REVOKE_EMOTE))
		stc("You're not feeling very emotional right now.\n\r", ch);

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS))
		stc("Your mouth seems to be stuck shut.\n\r", ch);

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY))
		stc("You only feel like flaming.\n\r", ch);
}

char *makedrunk(CHAR_DATA *ch, char *string)
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
	static char buf[MAX_STRING_LENGTH];
	char temp;
	int pos = 0;
	int drunklevel;
	int randomnum;

	/* Check how drunk a person is... */
	if (IS_NPC(ch))
		drunklevel = 0;
	else
		drunklevel = ch->pcdata->condition[COND_DRUNK];

	if (drunklevel <= 10)
		return string;

	/* drunk in earnest. mangle his speech. */
	for (; *string; string++) {
		temp = toupper(*string);

		if ((temp >= 'A') && (temp <= 'Z')) {
			if (drunklevel > drunk[temp - 'A'].min_drunk_level) {
				randomnum = number_range(0, drunk[temp - 'A'].number_of_rep);
				strcpy(&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
				pos += strlen(&buf[pos]);
			}
			else
				buf[pos++] = *string;
		}
		else if ((temp >= '0') && (temp <= '9')) {
			temp = '0' + number_range(0, 9);
			buf[pos++] = temp;
		}
		else
			buf[pos++] = *string;

		if (pos > (MAX_STRING_LENGTH - 25))
			break;
	}

	buf[pos] = '\0';          /* Mark end of the string... */
	return (buf);
} /* end makedrunk() */

/* This sends an ACT-type message to everybody in the game. */
void global_act(CHAR_DATA *ch, char *message,
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
			act_new(message, ch, NULL, d->character, TO_VICT, POS_SLEEPING, FALSE);
			set_color(victim, WHITE, NOBOLD);
		}
	}
} /* end global_act() */

bool swearcheck(char *argument)
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
	sprintf(tobechecked, "%s", smash_bracket(argument));;

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

bool check_channel_social(CHAR_DATA *ch, int channel, int custom, char *command, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
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
		stc("You are anti-social!\n\r", ch);
		return TRUE;
	}

	one_argument(argument, arg);
	victim = get_player_world(ch, arg, VIS_PLR);
	new_color(ch, custom);

	if (arg[0] == '\0') {
		if (iterator->char_no_arg != NULL)
			stc("[S] ", ch);

		act_new(iterator->char_no_arg,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}
	else if (victim == NULL) {
		stc("[S] They are not here.\n\r", ch);
		return TRUE;
	}
	else if (victim == ch) {
		if (iterator->char_auto != NULL)
			stc("[S] ", ch);

		act_new(iterator->char_auto,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}
	else {
		if (iterator->char_found != NULL)
			stc("[S] ", ch);

		act_new(iterator->char_found,  ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}

	if (chit_info) {
		char buf[MAX_INPUT_LENGTH];
		sprintf(buf, "%s social %d %s %d %s",
		        ch->name, ch->sex, command, victim ? victim->sex : 0, victim ? victim->name : argument);
		Write_To_Server(chit_info, "ALL", buf);
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

			if (arg[0] == '\0') {
				if (iterator->others_no_arg != NULL)
					stc("[S] ", vic);

				act_new(iterator->others_no_arg, ch, NULL, vic, TO_VICT, POS_SLEEPING, FALSE);
			}
			else if (victim == ch) {
				if (iterator->others_auto != NULL)
					stc("[S] ", vic);

				act_new(iterator->others_auto, ch, vic, victim, TO_WORLD, POS_SLEEPING, FALSE);
			}
			else if (vic == victim) {
				if (iterator->vict_found != NULL)
					stc("[S] ", vic);

				act_new(iterator->vict_found,  ch, NULL, victim, TO_VICT, POS_SLEEPING, FALSE);
			}
			else {
				if (iterator->others_found != NULL)
					stc("[S] ", vic);

				act_new(iterator->others_found, ch, vic, victim, TO_WORLD, POS_SLEEPING, FALSE);
			}

			set_color(vic, WHITE, NOBOLD);
		}
	}

	return TRUE;
}

/* Channel who by Lotus */
void channel_who(CHAR_DATA *ch, char *channelname, int channel, int custom)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch))
		return;

	new_color(ch, custom);
	ptc(ch, "Players with %s ON\n\r", channelname);

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

			ptc(ch, "%s\n\r", victim->name);
		}
	}

	return;
}

/* This routine is for query call statements, checks to see which users in
the query list are on, and then sends the mesg to them */
void send_to_query(CHAR_DATA *ch, char *string)
{
	PC_DATA *pc;
	int i;

	/* Didn't use get_char_world because I didn't want a "vis" check */
	for (pc = pc_list; pc; pc = pc->next) {
		if (!pc->ch
		    || IS_NPC(pc->ch)
		    || IS_SET(pc->ch->comm, COMM_NOQUERY)
		    || is_ignoring(pc->ch, ch))
			continue;

		for (i = 0; i < MAX_QUERY; i++) {
			if (!ch->pcdata->query[i]
			    || !is_name(ch->pcdata->query[i], pc->ch->name))
				continue;

			new_color(pc->ch, CSLOT_CHAN_QTELL);
			stc(string, pc->ch);
			set_color(pc->ch, WHITE, NOBOLD);
			break;
		}
	}
}

void send_to_clan(CHAR_DATA *ch, CLAN_DATA *target, char *text)
{
	DESCRIPTOR_DATA *d;

	if (target == NULL) {
		stc("No such clan!\n\r", ch);
		return;
	}

	if (text[0] == '\0') {
		stc("Tell them what ?\n\r", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (IS_PLAYING(d) &&
		    d->character->clan == target)
			stc(text, d->character);
	}
}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip, int min_rank)
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

			act_new(string, d->character, obj, ch, TO_CHAR, POS_DEAD, FALSE);
		}
	}
}

void channel(CHAR_DATA *ch, char *argument, int channel)
{
	char arg[MSL];
	DESCRIPTOR_DATA *d;
	int cslot = chan_table[channel].cslot;

	if (channel == CHAN_CLAN) {
		if (!is_clan(ch) || (ch->clan->independent && !IS_IMMORTAL(ch))) {
			stc("You aren't in a clan.\n\r", ch);
			return;
		}
	}

	if (argument[0] == '\0') {
		new_color(ch, cslot);

		if (IS_SET(ch->comm, chan_table[channel].bit)) {
			ptc(ch, "The %s channel is now ON.\n\r", chan_table[channel].name);
			REMOVE_BIT(ch->comm, chan_table[channel].bit);
		}
		else {
			ptc(ch, "The %s channel is now OFF.\n\r", chan_table[channel].name);
			SET_BIT(ch->comm, chan_table[channel].bit);
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET) && channel != CHAN_IMMTALK) {
		new_color(ch, cslot);
		stc("You must turn off QUIET mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY) && channel != CHAN_FLAME) {
		stc("The gods have restricted you to the flame channel.\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, chan_table[channel].revoke_bit)) {
		ptc(ch, "The gods have revoked your ability to use %s.\n\r", chan_table[channel].name);
		return;
	}

	if (get_position(ch) < POS_SLEEPING) {
		stc("You are hurt too bad for that.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (!str_cmp(arg, "who") && argument[3] == '\0') {
		channel_who(ch, chan_table[channel].name, chan_table[channel].bit, cslot);
		return;
	}

	REMOVE_BIT(ch->comm, chan_table[channel].bit);
	new_color(ch, cslot);

	if (channel == CHAN_IMMTALK) /* lil different for immtalk */
		ptc(ch, "%s{x%s: %s{x\n\r",
		    ch->name,
		    ch->secure_level == RANK_IMP ? " [{YIMP{x]" :
		    ch->secure_level == RANK_HEAD ? " [{BHEAD{x]" :
		    " [IMM]",
		    argument);
	else
		ptc(ch, "%s {x'%s{x'\n\r", chan_table[channel].prefix_self, argument);

	set_color(ch, WHITE, NOBOLD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) && d->character != ch) {
			/* If the target player is in another MUD, we do not want
			   them receiving messages. -- Outsider
			*/
			if (!IS_NPC(victim) && (victim->pcdata->phone_socket))
				continue;

			if (channel == CHAN_IMMTALK) { /* we can skip a lot of the below junk */
				if (!IS_IMMORTAL(victim)
				    || GET_RANK(victim) < ch->secure_level
				    || IS_SET(victim->comm, chan_table[channel].bit))
					continue;

				new_color(victim, cslot);
				ptc(victim, "%s{x%s: %s{x\n\r",
				    PERS(ch, victim, VIS_PLR),
				    ch->secure_level == RANK_IMP ? " [{YIMP{x]" :
				    ch->secure_level == RANK_HEAD ? " [{BHEAD{x]" :
				    " [IMM]",
				    argument);
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
				ptc(victim, "%s (%s) implores the gods: \"%s{x\"\n\r",
				    ch->desc->original->name, ch->name, argument);
			else
				/* -----> */                    act_new(chan_table[channel].other_str,
				                                        ch, argument, victim, TO_VICT_CHANNEL, POS_SLEEPING, FALSE);

			set_color(victim, WHITE, NOBOLD);
		}
	}

	/* If we are connected to a Chat server, send channel info. -- Outsider */
	if (chit_info) {
		char buffer[BUFFER_SIZE];

		if ((strlen(ch->name) + strlen(argument)) < BUFFER_SIZE) {
			/* for now, just send gossip */
			if (channel == CHAN_GOSSIP) {
				sprintf(buffer, "%s gossip %s", ch->name, argument);
				Write_To_Server(chit_info, "ALL", buffer);
			}
			else if (channel == CHAN_GRATS) {
				sprintf(buffer, "%s message %s@%s grats %s\n\r",
				        ch->name, ch->name, chit_info->username, argument);
				Write_To_Server(chit_info, "ALL", buffer);
			}
			else if (channel == CHAN_FLAME) {
				sprintf(buffer, "%s message %s@%s flames %s\n\r",
				        ch->name, ch->name, chit_info->username, argument);
				Write_To_Server(chit_info, "ALL", buffer);
			}
			else if (channel == CHAN_MUSIC) {
				sprintf(buffer, "%s message %s@%s musics %s\n\r",
				        ch->name, ch->name, chit_info->username, argument);
				Write_To_Server(chit_info, "ALL", buffer);
			}
			else if (channel == CHAN_IMMTALK) {
				sprintf(buffer, "%s immortal %d %s\n\r",
				        ch->name, ch->secure_level, argument);
				Write_To_Server(chit_info, "ALL", buffer);
			}
		}   /* end of size ok */
	}     /* end of sending data to Chat server */
}

void do_gossip(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_GOSSIP);
}

void do_flame(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_FLAME);
}

void do_qwest(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_QWEST);
}

void do_pray(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_PRAY);
}

void do_clantalk(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_CLAN);
}

void do_music(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_MUSIC);
}

void do_ic(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch)) {
		stc("Just be yourself, no need to pretend :)\n\r", ch);
		return;
	}

	if (!IS_SET(ch->pcdata->plr, PLR_OOC)) {
		stc("You are not in character (help ooc).\n\r", ch);
		return;
	}

	channel(ch, argument, CHAN_IC);
}

void do_grats(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_GRATS);
}

void do_immtalk(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_IMMTALK);
}

void do_question(CHAR_DATA *ch, char *argument)
{
	channel(ch, argument, CHAN_QA);
}

void talk_auction(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d)
		    && d->character != ch
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


/*
This function turns the remote messages on and off.
Without arguments, it displays the current status
of the remote channels. The remote channel may been turned
"on" or "off".
-- Outsider
*/
void do_remote(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	if (! argument[0]) {   /* display status */
		stc("Remote channels are now ", ch);

		if (ch->pcdata->block_remote) stc("OFF.\n\r", ch);
		else stc("ON.\n\r", ch);

		return;
	}

	if (! strcasecmp(argument, "on")) {   /* turn channel on */
		ch->pcdata->block_remote = FALSE;
		stc("Remote channels are now ON.\n\r", ch);
		return;
	}

	if (! strcasecmp(argument, "off")) {   /* turn channel off */
		ch->pcdata->block_remote = TRUE;
		stc("Remote channels are now OFF.\n\r", ch);
		return;
	}
}


void do_announce(CHAR_DATA *ch, char *argument)
{
	if (IS_SET(ch->comm, COMM_NOANNOUNCE)) {
		new_color(ch, CSLOT_CHAN_ANNOUNCE);
		stc("Announcements will now be shown.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		REMOVE_BIT(ch->comm, COMM_NOANNOUNCE);
	}
	else {
		new_color(ch, CSLOT_CHAN_ANNOUNCE);
		stc("Announce channel is now OFF.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		SET_BIT(ch->comm, COMM_NOANNOUNCE);
	}
}

void do_send_announce(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	char send_buffer[BUFFER_SIZE];

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d)
		    && d->character != ch
		    && !IS_SET(victim->comm, COMM_NOANNOUNCE)
		    && !IS_SET(victim->comm, COMM_QUIET)) {
			new_color(victim, CSLOT_CHAN_ANNOUNCE);
			ptc(victim, "[FYI] %s{x\n\r", argument);
			set_color(victim, WHITE, NOBOLD);
		}
	}

	/* Try to send announce to other MUDs. -- Outsider */
	if ((chit_info) && (strlen(argument) < (BUFFER_SIZE - 32))) {
		sprintf(send_buffer, "%s message {W[FYI{Y@{W%s] %s{x\n\r", ch->name,
		        chit_info->username, argument);
		Write_To_Server(chit_info, "ALL", send_buffer);
	}
}


/* Lotus - Let us Imms use the FYI Channel for jokes */
void do_fyi(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char send_buffer[BUFFER_SIZE];
	new_color(ch, CSLOT_CHAN_ANNOUNCE);
	ptc(ch, "You FYI '%s{x'\n\r", argument);
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
				act_new("[$n{x] $t{x",
				        ch, argument, d->character, TO_VICT, POS_SLEEPING, FALSE);
			else
				act_new("[FYI] $t{x",
				        ch, argument, d->character, TO_VICT, POS_SLEEPING, FALSE);

			set_color(ch, WHITE, NOBOLD);
		}
	}

	/* Try to send FYIs to other MUDs. -- Outsider */
	if ((chit_info) && (strlen(argument) < (BUFFER_SIZE - 32))) {
		sprintf(send_buffer, "%s message {W[FYI{Y@{W%s] %s{x",
		        ch->name, chit_info->username, argument);
		Write_To_Server(chit_info, "ALL", send_buffer);
	}
}

void do_replay(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch)) {
		stc("{YMobiles can't work answering machines.{x\n\r", ch);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	page_to_char(buf_string(ch->pcdata->buffer), ch);
	set_color(ch, WHITE, NOBOLD);
	clear_buf(ch->pcdata->buffer);
}

/* Channel specifically for socials and emotes by Lotus */
void do_globalsocial(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char *arg2;
	DESCRIPTOR_DATA *d;
	int pose = -1;

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm, COMM_NOSOCIAL)) {
			new_color(ch, CSLOT_CHAN_SOCIAL);
			stc("Social channel is now ON.\n\r", ch);
			REMOVE_BIT(ch->comm, COMM_NOSOCIAL);
			set_color(ch, WHITE, NOBOLD);
		}
		else {
			new_color(ch, CSLOT_CHAN_SOCIAL);
			stc("Social channel is now OFF.\n\r", ch);
			SET_BIT(ch->comm, COMM_NOSOCIAL);
			set_color(ch, WHITE, NOBOLD);
		}

		return;
	}

	/* social sent, turn Social on if it isn't already */
	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_SOCIAL);
		stc("You must turn off quiet mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_FLAMEONLY)) {
		stc("The gods have restricted you to the flame channel.\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_SOCIAL)) {
		stc("The gods have revoked your ability to use this channel.\n\r", ch);
		return;
	}

	arg2 = one_argument(argument, arg);

	if (!str_prefix1(arg, "who") && argument[3] == '\0') {
		channel_who(ch, "Social", COMM_NOSOCIAL, CSLOT_CHAN_SOCIAL);
		return;
	}

	REMOVE_BIT(ch->comm, COMM_NOSOCIAL);

	/* channel social: SOCIAL <social> */
	/* This checks for predefined socials. -- Outsider */
	if (check_channel_social(ch, COMM_NOSOCIAL, CSLOT_CHAN_SOCIAL, arg, arg2))
		return;

	pose = -1;

	/***** social pose removed -- Elrac
	if (!str_prefix1( arg, "pose" ))
	{
	    pose = select_pose( ch );
	    if ( pose < 0 ) return;
	    sprintf( buf, "[P] %s\n\r",
	        new_pose_table[ch->class].poses[pose].self_msg );
	}
	else
	*****/
	if (!str_prefix1(arg, "emote") && (arg2[0] != '\0'))
		sprintf(buf, "[E] %s %s\n\r", ch->name, arg2);
	else
		sprintf(buf, "You socialize '%s{x'\n\r", argument);

	/* send social to player himself */
	new_color(ch, CSLOT_CHAN_SOCIAL);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	/* set up message to send everybody */

	/* no channel poses */
	/*    if ( pose != -1 )
	        sprintf( buf, "[P] %s\n\r",
	            new_pose_table[ch->class].poses[pose].room_msg );
	*/

	if (!str_prefix1(arg, "emote") && (arg2[0] != '\0'))
		sprintf(buf, "[E] $n %s", arg2);
	else
		sprintf(buf, "$n socializes '%s{x'", argument);

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
			act_new(buf, ch, NULL, victim, TO_VICT, POS_SLEEPING, FALSE);
			set_color(victim, WHITE, NOBOLD);
		}
	}   /* end of for loop -- to each player */

	/* Send the social to other MUDs. -- Outsider */
	if (chit_info) {
		if ((! str_prefix1(arg, "emote")) && (arg2[0] != '\0'))
			sprintf(buf, "%s message [E] %s@%s %s\n\r",
			        ch->name, ch->name, chit_info->username, arg2);
		else
			sprintf(buf, "%s social %d %s",
			        ch->name, ch->sex, argument);

		Write_To_Server(chit_info, "ALL", buf);
	}
}

void do_iclantalk(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	CLAN_DATA *clan, *oclan;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		new_color(ch, CSLOT_CHAN_CLAN);
		stc("Syntax: iclan <clan name> <message>\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((clan = clan_lookup(arg)) == NULL) {
		stc("No such clan exists, type clanlist for a list of valid clans.\n\r", ch);
		return;
	}

	oclan = ch->clan;
	ch->clan = clan;
	ptc(ch, "%s ", clan->who_name);
	do_clantalk(ch, argument);
	ch->clan = oclan;
}

/* Improved do_say (With Colour!) - Xenith */
void do_say(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	if (ch->in_room == NULL)
		return;

	if (argument[0] == '\0') {
		stc("Say what?\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_SAY)) {
		stc("You are unable to talk!\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)) {
		stc("A heavy mist dampens all sound in the room.\n\r", ch);
		return;
	}

	argument = makedrunk(ch, argument);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch != ch && IS_AWAKE(vch)) {
			new_color(vch, CSLOT_CHAN_SAY);

			/* make say command handle yelling and questions */
			if (argument[ strlen(argument) - 1] == '?')
				ptc(vch, "%s asks '%s'\n\r", PERS(ch, vch, VIS_CHAR), argument);
			else if (argument[ strlen(argument) - 1] == '!')
				ptc(vch, "%s exclaims '%s'\n\r", PERS(ch, vch, VIS_CHAR), argument);
			else
				ptc(vch, "%s says '%s'\n\r", PERS(ch, vch, VIS_CHAR), argument);

			set_color(vch, WHITE, NOBOLD);
		}
	}

	/* viewing room stuff */
	/* make say command handle yelling and questions */
	if (argument[ strlen(argument) - 1] == '?')
		act("$n asks '$T'\n\r", ch, NULL, argument, TO_VIEW);
	else if (argument[ strlen(argument) - 1] == '!')
		act("$n exclaims '$T'\n\r", ch, NULL, argument, TO_VIEW);
	else
		act("$n says '$T'\n\r", ch, NULL, argument, TO_VIEW);

	new_color(ch, CSLOT_CHAN_SAY);

	/* one more time...make say command handle questions and yelling */
	if (argument[ strlen(argument) - 1] == '?')
		ptc(ch, "You ask '%s'\n\r", argument);
	else if (argument[ strlen(argument) - 1] == '!')
		ptc(ch, "You exclaim '%s'\n\r", argument);
	else
		ptc(ch, "You say '%s'\n\r", argument);

	set_color(ch, WHITE, NOBOLD);
	mprog_speech_trigger(argument, ch);
}


void do_tell(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL];
	CHAR_DATA *victim;
	char *strtime;

	if (IS_SET(ch->revoke, REVOKE_TELL)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("The Imms have rescinded your ability to TELL.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("You must turn off quiet mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_DEAF)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("You must turn off deaf mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Tell whom what?\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* Check to see if the receiver is on a different MUD. -- Outsider */
	if (strchr(arg, '@')) {
		char target_mud[BUFFER_SIZE], target_player[BUFFER_SIZE];
		char buffer_to_send[BUFFER_SIZE];
		int index = 0;

		if (! chit_info) {
			new_color(ch, CSLOT_CHAN_TELL);
			stc("You are not connected to other MUDs.\r\n", ch);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

		/* seperate player name from target mud name */
		while (arg[index] != '@') {
			target_player[index] = arg[index];
			index++;
		}

		target_player[index] = '\0';    /* put NULL at end of string */
		index++;   /* go to first char after the @ */
		strcpy(target_mud, & (arg[index]));    /* copy remainder of target into MUD name */

		/* make sure text will fit in buffer */
		if ((strlen(ch->name) + strlen(target_player) + strlen(argument) + 16) >= BUFFER_SIZE)
			return;

		/* create command to send to the Chat server */
		sprintf(buffer_to_send, "%s tell %s %s", ch->name, target_player, argument);
		Write_To_Server(chit_info, target_mud, buffer_to_send);
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Message sent.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}    /* end of sending tell to other MUD */

	/* Can tell to PC's anywhere, but NPC's only in same room. */
	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
		if ((victim = get_mob_here(ch, arg, VIS_CHAR)) == NULL) {
			new_color(ch, CSLOT_CHAN_TELL);
			stc("They aren't here.\n\r", ch);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

	if (IS_SET(victim->comm, COMM_QUIET | COMM_DEAF | COMM_NOCHANNELS) && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E is not receiving tells.", ch, 0, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		ptc(ch, "%s is ignoring you.\n\r", victim->name);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (victim->desc == NULL && !IS_NPC(victim)) {
		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E has lost $S link, but your message will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		sprintf(buf, "[%s] %s tells you '%s{x'\n\r", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	if (IS_SET(victim->comm, COMM_AFK)) {
		if (IS_NPC(victim)) {
			new_color(ch, CSLOT_CHAN_TELL);
			act_new("$E is AFK, and not receiving tells.",
			        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E is AFK, but your tell will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		stc(victim->pcdata->afk, ch);
		set_color(ch, WHITE, NOBOLD);
		sprintf(buf, "[%s] %s tells you '%s{x'\n\r", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	/* If the target is on another MUD, they should not hear tells. -- Outsider */
	if ((! IS_NPC(victim)) && (victim->pcdata->phone_socket)) {
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E is off in another world and cannot hear you.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	act_new("You tell $N '$t{x'", ch, argument, victim, TO_CHAR, POS_SLEEPING, FALSE);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_TELL);
	act_new("$n tells you '$t{x'", ch, argument, victim, TO_VICT, POS_SLEEPING, FALSE);
	set_color(victim, WHITE, NOBOLD);

	if (IS_NPC(victim) || !victim->replylock)
		strcpy(victim->reply, ch->name);
}

void do_reply(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char *strtime;
	bool found = FALSE;

	if (argument[0] == '\0') {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Reply with what?\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_TELL)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("Your message didn't get through.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (ch->reply[0] == '\0') {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("But no one has sent you a tell yet!\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		ch->replylock = FALSE;
		return;
	}

	/* Check to see if we should send the reply to another MUD. -- Outsider */
	if (strchr(ch->reply, '@')) {
		new_color(ch, CSLOT_CHAN_TELL);

		/* If we have a connection, send the reply to another MUD. */
		if (chit_info) {
			char my_buffer[BUFFER_SIZE];
			char player_name[BUFFER_SIZE], mud_name[BUFFER_SIZE];
			int index;
			/* copy the player name (up to the @) into player_name */
			index = 0;

			while (ch->reply[index] != '@') {
				player_name[index] = ch->reply[index];
				index++;
			}

			player_name[index] = '\0';    /* put terminating NULL on player_name */
			index++;           /* goto begining of mud name */
			strcpy(mud_name, & (ch->reply[index]));
			sprintf(my_buffer, "%s tell %s %s", ch->name, player_name, argument);
			Write_To_Server(chit_info, mud_name, my_buffer);
			stc("Message sent.\n\r", ch);
		}
		else
			stc("Message could not be sent to other MUD.\n\r", ch);

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	for (victim = char_list; victim != NULL ; victim = victim->next)
		if (! strcmp(ch->reply, victim->name)) {
			found = TRUE;
			break;
		}

	if (!found) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("They aren't here.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		ch->replylock = FALSE;
		return;
	}

	/* Lock replies to someone, idea by Scattercat */
	if (!str_cmp(argument, "lock") && !IS_NPC(ch)) {
		ch->replylock = !ch->replylock;

		if (!ch->replylock)
			ptc(ch, "You unlock your replies from %s.\n\r", PERS(victim, ch, VIS_PLR));
		else
			ptc(ch, "You lock your replies to %s.\n\r", PERS(victim, ch, VIS_PLR));

		return;
	}

	if ((IS_SET(victim->comm, COMM_QUIET)
	     || IS_SET(victim->comm, COMM_DEAF)
	     || IS_SET(victim->comm, COMM_NOCHANNELS))
	    && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E is not receiving tells.", ch, 0, victim, TO_CHAR, POS_DEAD, FALSE);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_TELL);
		ptc(ch, "%s is ignoring you.\n\r", victim->name);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (victim->desc == NULL && !IS_NPC(victim)) {
		strtime = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act("$N seems to have misplaced $S link...try again later.", ch, NULL, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		sprintf(buf, "[%s] %s tells you '%s{x'\n\r", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	if (IS_SET(victim->comm, COMM_AFK)) {
		if (IS_NPC(victim)) {
			new_color(ch, CSLOT_CHAN_TELL);
			act_new("$E is AFK, and not receiving tells.",
			        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
			set_color(ch, WHITE, NOBOLD);
			return;
		}

		strtime = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		new_color(ch, CSLOT_CHAN_TELL);
		act_new("$E is AFK, but your tell will go through when $E returns.",
		        ch, NULL, victim, TO_CHAR, POS_DEAD, FALSE);
		act(victim->pcdata->afk, ch, NULL, NULL, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		sprintf(buf, "[%s] %s tells you '%s{x'\n\r", strtime, PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer, buf);
		return;
	}

	new_color(ch, CSLOT_CHAN_TELL);
	act_new("You tell $N '$t{x'", ch, argument, victim, TO_CHAR, POS_DEAD, FALSE);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_TELL);
	act_new("$n tells you '$t{x'", ch, argument, victim, TO_VICT, POS_DEAD, FALSE);
	set_color(victim, WHITE, NOBOLD);

	if (IS_NPC(victim))
		strcpy(victim->reply, ch->name);
	else if (!victim->replylock)
		strcpy(victim->reply, ch->name);
}

void do_yell(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		stc("You yell your head off but no one hears.\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_NOCHANNELS)) {
		stc("The gods have revoked your channel priviliges.\n\r", ch);
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

void do_emote(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_EMOTE)) {
		stc("You're not feeling very emotional right now.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("Emote what?\n\r", ch);
		return;
	}

	act("$n $T{x", ch, NULL, argument, TO_ROOM);
	act("$n $T{x", ch, NULL, argument, TO_CHAR);
	mprog_speech_trigger(argument, ch);
	return;
}


void do_pmote(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_EMOTE)) {
		stc("You're not feeling very emotional right now.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("Emote what?\n\r", ch);
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
		name = vch->name;

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
					name = vch->name;
					continue;
				}

				strncat(last, letter, 1);
				continue;
			}

			matches = 0;
			strcat(temp, last);
			strncat(temp, letter, 1);
			last[0] = '\0';
			name = vch->name;
		}

		act("$N $t{x", vch, temp, ch, TO_CHAR);
	}

	return;
}

void do_smote(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS)) {
		stc("You cannot show your emotions.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("S-Emote what?\n\r", ch);
		return;
	}

	if (strstr(argument, ch->name) == NULL) {
		stc("You must include your name in an s-emote.\n\r", ch);
		return;
	}

	stc(argument, ch);
	stc("\n\r", ch);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			stc(argument, vch);
			stc("\n\r", vch);
			continue;
		}

		strcpy(temp, argument);
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;

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
					name = vch->name;
					continue;
				}

				strncat(last, letter, 1);
				continue;
			}

			matches = 0;
			strcat(temp, last);
			strncat(temp, letter, 1);
			last[0] = '\0';
			name = vch->name;
		}

		stc(temp, vch);
		stc("\n\r", vch);
	}

	return;
}

void do_page(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	char *strtime;

	if (IS_SET(ch->revoke, REVOKE_PAGE)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("The Imms have rescinded your ability to PAGE.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("You must turn off quiet mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (IS_SET(ch->comm, COMM_DEAF)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		stc("You must turn off deaf mode first.\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		new_color(ch, CSLOT_CHAN_PAGE);

		if (IS_SET(ch->comm, COMM_NOPAGE)) {
			stc("You turn on your pager.\n\r", ch);
			REMOVE_BIT(ch->comm, COMM_NOPAGE);
		}
		else {
			stc("You turn off your pager.\n\r", ch);
			SET_BIT(ch->comm, COMM_NOPAGE);
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		new_color(ch, CSLOT_CHAN_PAGE);
		ptc(ch, "You failed to locate [%s] in these realms.\n\r", arg);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if ((IS_SET(victim->comm, COMM_NOPAGE))
	    && !IS_IMMORTAL(ch)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		act("$E does not have $S pager turned on.", ch, 0, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (is_ignoring(victim, ch)) {
		new_color(ch, CSLOT_CHAN_PAGE);
		ptc(ch, "%s is ignoring you.\n\r", victim->name);
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
		sprintf(buf, "{R[%s] %s PAGES '%s{x'{x\a\n\r",
		        strtime , PERS(ch, victim, VIS_PLR), argument);
		buf[0] = UPPER(buf[0]);

		if (!IS_NPC(victim)) add_buf(victim->pcdata->buffer, buf);

		return;
	}

	new_color(ch, CSLOT_CHAN_PAGE);
	act_new("You PAGE $N '$t{x'", ch, argument, victim, TO_CHAR, POS_SLEEPING, FALSE);
	stc("Thank you for using Comcast Telecommunications, Inc.\n\r", ch);
	set_color(ch, WHITE, NOBOLD);
	new_color(victim, CSLOT_CHAN_PAGE);
	act_new("$n PAGES '$t{x'\a", ch, argument, victim, TO_VICT, POS_SLEEPING, FALSE);
	set_color(victim, WHITE, NOBOLD);
	strcpy(victim->reply, ch->name);
	return;
}

void do_whisper(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if (IS_NPC(ch)) {
		stc("You lack the refined delicacy of voice to whisper.\n\r", ch);
		return;
	}

	if (ch->pcdata->whisper == NULL) {
		do_huh(ch);
		return;
	}

	if ((victim = get_player_world(ch, ch->pcdata->whisper, VIS_PLR)) == NULL) {
		ptc(ch, "{G%s isn't here. :({x\n\r", ch->pcdata->whisper);
		return;
	}

	if (argument[0] == '\0') {
		ptc(victim, "{G%s whispers sweet nothings into your ear.{x\n\r", ch->name);
		ptc(ch, "{GYou whisper sweet nothings into %s's ear.{x\n\r", victim->name);
		return;
	}

	ptc(victim, "{G%s whispers in your ear, '%s{G'{x\n\r", ch->name, argument);
	ptc(ch, "{GYou whisper in %s's ear, '%s{G'{x\n\r", victim->name, argument);
}

void do_qtell(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		stc("You do not have a query list.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm, COMM_NOQUERY)) {
			new_color(ch, CSLOT_CHAN_QTELL);
			stc("Query channel is now on.\n\r", ch);
			REMOVE_BIT(ch->comm, COMM_NOQUERY);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
		else {
			new_color(ch, CSLOT_CHAN_QTELL);
			stc("Query channel is now off.\n\r", ch);
			SET_BIT(ch->comm, COMM_NOQUERY);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
	}

	if (!ch->pcdata->query) {
		stc("You have no one on your query list.\n\r", ch);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_QTELL)) {
		stc("Your message didn't get through!\n\r", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOQUERY)) {
		new_color(ch, CSLOT_CHAN_QTELL);
		stc("Query channel is now on.\n\r", ch);
		REMOVE_BIT(ch->comm, COMM_NOQUERY);
		set_color(ch, WHITE, NOBOLD);
	}

	new_color(ch, CSLOT_CHAN_QTELL);
	ptc(ch, "You tell the query '%s{x'\n\r", argument);
	set_color(ch, WHITE, NOBOLD);
	sprintf(buf, "%s tells the query '%s{x'\n\r", ch->name, argument);
	send_to_query(ch, buf);
	return;
}

void do_gtell(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;

	if (argument[0] == '\0') {
		new_color(ch, CSLOT_CHAN_GTELL);
		stc("Tell your group what?\n\r", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/*
	 * Note use of stc, so gtell works on sleepers.
	 */
	sprintf(buf, "%s tells the group '%s{x'\n\r", ch->name, argument);

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

void do_xsocial(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim = NULL;
	bool found = FALSE;
	bool foundwrong = FALSE;
	struct xsocial_type *iterator;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch)) {
		stc("Mobiles lack the stamina to xsocial.\n\r", ch);
		return;
	}

	if (GET_SEX(ch) == SEX_NEUTRAL) {
		stc("Only men and women can xsocial.\n\r", ch);
		return;
	}

	if (IS_SET(ch->censor, CENSOR_XSOC)) {
		stc("You must turn off censor xsocials first.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0') {
		stc("Xsocial what?\n\r", ch);
		return;
	}

	for (iterator = xsocial_table_head->next; iterator != xsocial_table_tail; iterator = iterator->next) {
		if (!str_prefix1(arg1, iterator->name)) {
			foundwrong = TRUE;

			if (iterator->csex == 0
			    || iterator->csex == GET_SEX(ch)) {
				found = TRUE;
				break;
			}
		}
	}

	if (!found) {
		if (foundwrong) {
			if (GET_SEX(ch) == SEX_MALE)
				stc("Only women can perform that xsocial.\n\r", ch);
			else
				stc("Only men can perform that xsocial.\n\r", ch);
		}
		else
			stc("That is not a valid xsocial.\n\r", ch);

		return;
	}

	switch (get_position(ch)) {
	case POS_DEAD:
		stc("This is NOT the night of the living DEAD.\n\r", ch);
		return;

	case POS_INCAP:
	case POS_MORTAL:
		stc("You are hurt far too bad for that.\n\r", ch);
		return;

	case POS_STUNNED:
		stc("You are too stunned to do that.\n\r", ch);
		return;

	case POS_SLEEPING:
		stc("Stop fidgeting and get some sleep!\n\r", ch);
		return;
	}

	/* no argument, cnoarg and onoarg */
	if (arg2[0] == '\0') {
		xact(iterator->others_no_arg, ch, NULL, victim, TO_ROOM);
		xact(iterator->char_no_arg,   ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((victim = get_char_here(ch, arg2, FALSE)) == NULL) {
		stc("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Mobiles lack the stamina to xsocial.\n\r", ch);
		return;
	}

	if (GET_SEX(victim) == SEX_NEUTRAL) {
		stc("Only men and women can xsocial.\n\r", ch);
		return;
	}

	if (iterator->vsex != 0 && iterator->vsex != GET_SEX(victim)) {
		ptc(ch, "This xsocial can only be performed on a %s.\n\r",
		    iterator->vsex == SEX_FEMALE ? "woman" : "man");
		return;
	}

	/* victim is ch, cself and oself */
	if (victim == ch) {
		xact(iterator->others_auto, ch, NULL, victim, TO_ROOM);
		xact(iterator->char_auto,   ch, NULL, victim, TO_CHAR);
		return;
	}

	/* victim found, ofound, cfound, vfound */
	if (victim->pcdata->partner == NULL || str_cmp(victim->pcdata->partner, ch->name)) {
		stc("They have not given you permission.\n\r", ch);
		return;
	}

	if (IS_SET(victim->censor, CENSOR_XSOC)) {
		stc("Legacy is not set to {PX{x for them.\n\r", ch);
		return;
	}

	ch->pcdata->partner = victim->name;
	xact(iterator->others_found, ch, NULL, victim, TO_NOTVICT);
	xact(iterator->char_found,   ch, NULL, victim, TO_CHAR);
	xact(iterator->vict_found,   ch, NULL, victim, TO_VICT);
}

void do_query(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	bool found = FALSE;
	int pos;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);
	smash_tilde(arg2);

	if (str_cmp(arg, "+") && str_cmp(arg, "-")
	    && str_prefix1(arg, "list") && str_prefix1(arg, "clear")) {
		stc("Syntax:\n\r", ch);
		stc("{RQuery list{x       : Lists names in your query.\n\r", ch);
		stc("{RQuery +{x <{YName{x>   : Adds a name to your query.\n\r", ch);
		stc("{RQuery -{x <{YName{x>   : Removes a name from query.\n\r", ch);
		stc("{RQuery clear{x      : Clears your query (removes all names).\n\r", ch);
		stc(" ", ch);
		stc("{RQtell{x            : Turns your query channel on or off.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "list")) {
		if (rch->pcdata->query[0] == NULL) {
			stc("You have no one in your query.\n\r", ch);
			return;
		}

		stc("People in your query:\n\r", ch);

		for (pos = 0; pos < MAX_QUERY; pos++) {
			if (rch->pcdata->query[pos] == NULL)
				break;

			ptc(ch, "[%d] %s\n\r", pos, rch->pcdata->query[pos]);
		}

		return;
	}

	if (!str_prefix1(arg, "clear")) {
		for (pos = 0; pos < MAX_QUERY; pos++) {
			if (rch->pcdata->query[pos] == NULL)
				break;

			rch->pcdata->query[pos] = NULL;
		}

		stc("Query cleared.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "+")) {
		if (arg2[0] == '\0') {
			stc("Add who to the query list?\n\r", ch);
			return;
		}

		for (pos = 0; pos < MAX_QUERY; pos++) {
			if (rch->pcdata->query[pos] == NULL)
				break;

			if (!str_prefix1(arg2, rch->pcdata->query[pos])) {
				stc("That name is already in the query.\n\r", ch);
				return;
			}
		}

		if (pos >= MAX_QUERY) {
			stc("Too many users in query, remove a name.\n\r", ch);
			return;
		}

		free_string(rch->pcdata->query[pos]);
		rch->pcdata->query[pos]            = str_dup(arg2);
		ptc(ch, "%s has been added to the query list.\n\r",
		    rch->pcdata->query[pos]);
		return;
	}

	if (!str_prefix1(arg, "-")) {
		if (arg2 == '\0') {
			stc("Remove who from the query list?\n\r", ch);
			return;
		}

		for (pos = 0; pos < MAX_QUERY; pos++) {
			if (rch->pcdata->query[pos] == NULL)
				break;

			if (found) {
				rch->pcdata->query[pos - 1]           = rch->pcdata->query[pos];
				rch->pcdata->query[pos]             = NULL;
				continue;
			}

			if (!strcmp(arg2, rch->pcdata->query[pos])) {
				stc("Name removed from the query.\n\r", ch);
				free_string(rch->pcdata->query[pos]);
				rch->pcdata->query[pos] = NULL;
				found = TRUE;
			}
		}

		if (!found)
			stc("That name was not found in the query list.\n\r", ch);
	}
}



/*
This function gets called when a user on another MUD sends a gossip.
A gossip message in the format of "user@host gossips 'message'".
*/
void do_remote_gossip(char *from_mud, char *from_player, char *message_text)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char my_message[BUFFER_SIZE];

	if (strlen(from_mud) + strlen(from_player) + strlen(message_text) + 32 >= BUFFER_SIZE)
		return;

	sprintf(my_message, "{H%s{Y@{H%s gossips '%s{H'{x\n\r", from_player, from_mud, message_text);

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d)) {
			/* make sure we can send to this person */
			if (IS_SET(victim->comm, chan_table[CHAN_GOSSIP].bit) ||
			    IS_SET(victim->comm, COMM_QUIET))
				continue;

			if (victim->pcdata->block_remote)
				continue;

			stc(my_message, victim);
		}   /* end of is playing */
	}
}



/*
This function prints a message to one user's screen. The format
of "data" should be as follows:
<to_username> <message>
If we do not have a valid username, then the function should
simply return.
-- Outsider
*/

void do_remote_print(char *from_mud, char *from_player, char *data)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char target_name[BUFFER_SIZE];
	/* get the target user */
	data = one_argument(data, target_name);

	/* go through all users, looking for a matching name */
	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if ((IS_PLAYING(d)) && (!strcasecmp(target_name, victim->name))) {
			/* check for message blocking */
			if (victim->pcdata->block_remote)
				return;

			/* got match, print message */
			stc(data, victim);
		}
	}     /* end of for loop */
}

/*
This function sends a message to all active players.
The message to be displayed is passed in via "data".
-- Outsider
*/
void do_remote_message(char *from_mud, char *from_player, char *data)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	/* go through all players, looking for active ones */
	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if ((IS_PLAYING(d)) &&
		    (!IS_SET(victim->comm, COMM_NOANNOUNCE)) &&
		    (!IS_SET(victim->comm, COMM_QUIET))) {
			if (victim->pcdata->block_remote)
				continue;

			stc(data, victim);
		}
	}   /* end of for loop */
}



/*
This function handles "tell" commands sent in from the
Chat server. It searches for a user on this MUD with
a matching username. Then, if a user is found, it sends
that user a tell message.
"data" contains the target player and the text for
them to receive.
-- Outsider
*/
void do_remote_tell(char *from_mud, char *from_player, char *data)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim = NULL;    /* make compiler happy */
	char target_name[BUFFER_SIZE];
	bool found = FALSE;
	char big_long_output[BUFFER_SIZE];
	/* get the target username */
	data = one_argument(data, target_name);
	/* search for users with a matching name */
	d = descriptor_list;

	while ((!found) && (d != NULL)) {
		victim = d->original ? d->original : d->character;

		if ((IS_PLAYING(d)) && (! strcasecmp(target_name, victim->name)))
			found = TRUE;
		else
			d = d->next;
	}

	if (found) { /* if we have a match try to send them the tell */
		if (IS_SET(victim->comm, COMM_QUIET | COMM_DEAF | COMM_NOCHANNELS | COMM_AFK))
			return;

		if (victim->pcdata->block_remote)
			return;

		sprintf(big_long_output, "{B%s{Y@{x{B%s tells you '%s'{x\n\r", from_player, from_mud, data);
		stc(big_long_output, victim);

		/* Save the reply address. -- Outsider */
		if ((IS_NPC(victim)) || (!victim->replylock))
			sprintf(victim->reply, "%s@%s", from_player, from_mud);
	}

	return;
}


/*
This function accepts imm talk from another MUD and
sends the text to any immortal players who happen to
be on.
Data is passed in as
<security level> <message>

If an imm has a lower security level than the message
they don't get to see it.

-- Outsider (now engaged to Teotwawki)
*/
void do_remote_immtalk(char *from_mud, char *from_player, char *data)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	int security_level = 0;
	char secure_level[BUFFER_SIZE];
	data = one_argument(data, secure_level);
	sscanf(secure_level, "%d", &security_level);
	/* re-use secure_level to display the output */
	sprintf(secure_level, "{P%s{Y@{P%s [IMM]: %s{x",
	        from_player, from_mud, data);

	/* go through all players, looking for active ones */
	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if ((IS_PLAYING(d)) &&
		    (!IS_SET(victim->comm, COMM_NOANNOUNCE)) &&
		    (!IS_SET(victim->comm, COMM_QUIET))) {
			if (victim->pcdata->block_remote) continue;

			if (! IS_IMMORTAL(victim)) continue;

			if (victim->level < security_level) continue;

			stc(secure_level, victim);
		}
	}   /* end of for loop */
}


void do_remote_social(char *from_mud, char *from_player, char *data)
{
	char command[MIL];
	char arg[MIL];
	char victim[MIL];
	char buf[MSL];
	DESCRIPTOR_DATA *d;
	struct social_type *iterator;
	bool found = FALSE;
	int ch_sex, vic_sex;
	char gender[MSL], vic_gender[MSL];
	data = one_argument(data, gender);
	ch_sex = atoi(gender);
	sprintf(buf, "{G%s{Y@{G%s socializes '%s'{x\n\r", from_player, from_mud, data);
	data = one_argument(data, command);

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		if ((command[0] == iterator->name[0]) &&
		    (!str_prefix(command, iterator->name))) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		do_remote_message(from_mud, from_player, buf);
		return;
	}

	data = one_argument(data, arg);
	data = one_argument(data, victim);
	data = one_argument(data, vic_gender);
	vic_sex = atoi(vic_gender);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *vic;
		vic = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING &&
		    !IS_SET(vic->comm, COMM_NOSOCIAL) &&
		    !IS_SET(vic->comm, COMM_QUIET)) {
			/* avoid prefix on social messages when remote channel is blocked -- Outsider */
			if (IS_NPC(vic)) return;

			if (vic->pcdata->block_remote) return;

			if (victim[0] == '\0' || arg[0] == '\0') {
				if (iterator->others_no_arg != NULL) {
					sprintf(buf, "{G[%s{Y@{G%s] ", from_player, from_mud);
					stc(buf, vic);
				}

				remote_act_format(iterator->others_no_arg, from_player, victim, NULL, ch_sex,
				                  vic_sex, vic);
			}
			else if (is_name(victim, from_player)) {
				if (iterator->others_auto != NULL) {
					sprintf(buf, "{G[%s{Y@{G%s] ", from_player, from_mud);
					stc(buf, vic);
				}

				remote_act_format(iterator->others_auto, from_player, victim, NULL, ch_sex,
				                  vic_sex, vic);
			}
			else if (is_name(victim, vic->name)) {
				if (iterator->vict_found != NULL) {
					sprintf(buf, "{G[%s{Y@{G%s] ", from_player, from_mud);
					stc(buf, vic);
				}

				remote_act_format(iterator->vict_found, from_player, victim, NULL, ch_sex, vic_sex, vic);
			}
			else {
				if (iterator->others_found != NULL) {
					sprintf(buf, "{G[%s{Y@{G%s] ", from_player, from_mud);
					stc(buf, vic);
				}

				remote_act_format(iterator->others_found, from_player, victim, NULL, ch_sex, vic_sex, vic);
			}

			set_color(vic, WHITE, NOBOLD);
		}
	}

	return;
}

// Rehash of act_format and act_new to allow for remote socials
void remote_act_format(const char *format, char *ch, char *vch, const void *arg1, int ch_sex, int vic_sex,
                       CHAR_DATA *to)
{
	static char *const he_she  [] = { "it",  "he", "she" };
	static char *const him_her [] = { "it",  "him", "her" };
	static char *const his_her [] = { "its", "his", "her" };
	char buf[MAX_STRING_LENGTH * 100];
	const char *str;
	const char *i;
	char *point;
	char dollarmsg[3];
	point   = buf;
	str     = format;

	while (*str != '\0') {
		if (*str != '$') {
			*point++ = *str++;
			continue;
		}

		/* '$' sign after this point */
		++str;
		sprintf(dollarmsg, "$%c", *str);
		i = dollarmsg;

		switch (*str) {
		default:
			break;

		/* The following codes need 'ch', which should always be OK */

		case 'c': i = capitalize(ch); break;

		case 'n': i = ch; break;

		case 'e': i = he_she  [URANGE(0, ch_sex, 2)]; break;

		case 'm': i = him_her [URANGE(0, ch_sex, 2)]; break;

		case 's': i = his_her [URANGE(0, ch_sex, 2)]; break;

		/* The following codes need 'vch' */

		case 'N':
			if (vch[0] == '\0') {
				bug("Missing vch for '$$N'", 0);
				// bug( format, 0);  // This will cause an endless loop
				return;
			}
			else
				i = vch;

			break;

		case 'E':
			if (vch[0] == '\0') {
				bug("Missing vch for '$$E'", 0);
				return;
			}
			else
				i = he_she[ URANGE(0, vic_sex, 2) ];

			break;

		case 'M':
			if (vch[0] == '\0') {
				bug("Missing vch for '$$M'", 0);
				return;
			}
			else
				i = him_her[URANGE(0, vic_sex, 2)];

			break;

		case 'S':
			if (vch[0] == '\0') {
				bug("Missing vch for '$$S'", 0);
				return;
			}
			else
				i = his_her[URANGE(0, vic_sex, 2)];

			break;

		/* The following codes need no checking */

		case 'G': i = "\007";
			break;

		case '$': i = "$";
			break;
		}

		++str;

		while ((*point = *i) != '\0')
			++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	*point   = 0;
	buf[0]   = UPPER(buf[0]);

	/* make sure the receiver isn't blocking remote messages */
	if (to && to->desc) {
		if (IS_NPC(to)) return;

		if (to->pcdata->block_remote) return;

		stc(buf, to);
	}
} /* end remote_act_format() */


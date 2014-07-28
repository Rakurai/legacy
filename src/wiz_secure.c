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
* IMM security commands and related functions.   *
* Most of these are in the secure command group. *
*************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <mysql.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "mysql.h"


DECLARE_DO_FUN(do_quit		);	/* do_deny */
DECLARE_DO_FUN(do_echo		);
DECLARE_DO_FUN(do_fuckoff	);

void	show_list_to_char	args((OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, bool insidecont));


void do_fod(CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	CHAR_DATA *victim = NULL;
	char *msg = "In the distance you hear the thundering roar of a Finger Of Death!";
	ROOM_INDEX_DATA *ch_room, *victim_room;
	OBJ_DATA *on;

	if (argument[0] == '\0')
	{
		global_act(ch, msg, TRUE, YELLOW, COMM_QUIET|COMM_NOSOCIAL);
		stc("Your FOD reverberates through the world as a warning to all.\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (!OUTRANKS(ch, victim))
	{
		stc("They wouldn't like that.\n\r", ch);
		return;
	}

	if (victim->desc != NULL && victim->desc->original != NULL)
	{
		stc("Your victim is morphed or switched - no can do!\n\r", ch);
		return;
	}

	if (victim->fighting)
		stop_fighting(victim, TRUE);

	/* go to victim so bystanders will get local ACT */
	ch_room = ch->in_room;
	victim_room = victim->in_room;

	if (ch_room && victim_room && ch_room != victim_room)
	{
		on = ch->on;
		char_from_room(ch);
		char_to_room(ch, victim_room);
	}

	/* do it! */
	global_act(ch, msg, TRUE, YELLOW, COMM_QUIET|COMM_NOSOCIAL);
	act("You strike $N down with your {YFinger of {RDeath{x!", ch, NULL, victim, TO_CHAR);
	act("$n strikes $N down with a {YFinger of {RDeath{x!", ch, NULL, victim, TO_NOTVICT);
	act("$n strikes you down with a {YFinger of {RDeath{x!", ch, NULL, victim, TO_VICT);
	sprintf(buf, "$n has struck down %s!", victim->name);
	global_act(ch, buf, TRUE, YELLOW, COMM_QUIET|COMM_NOSOCIAL);
	act("You fall to the ground, dazed.", ch, NULL, victim, TO_VICT);
	victim->position = POS_RESTING;
	WAIT_STATE(victim, 15 * PULSE_PER_SECOND);

	/* return to previous room */
	if (ch_room && victim_room && ch_room != victim_room)
	{
		char_from_room(ch);
		char_to_room(ch, ch_room);
		on = ch->on;
	}
}


void do_force(CHAR_DATA *ch, char *argument)
{
	char buf[MSL], arg[MIL], arg2[MIL], arg3[MIL];
	PC_DATA *vpc, *vpc_next;
	CHAR_DATA *victim;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  force <player> <command>\n\r", ch);

		if (IS_IMP(ch))
			stc("  force all      <command>\n\r"
			    "  force mortals  <command>\n\r"
			    "  force gods     <command>\n\r", ch);

		return;
	}

	one_argument(argument,arg2);

	if (!str_cmp(arg2,"delete"))
	{
		stc("Do NOT do that.\n\r",ch);
		return;
	}

	one_argument(argument, arg3);

	sprintf(buf, "$n forces you to %s.", argument);

	if (IS_IMP(ch))
	{
		bool found = FALSE;

		if (!str_cmp(arg, "all"))
		{
			for (vpc = pc_list; vpc != NULL; vpc = vpc_next)
			{
				vpc_next = vpc->next;

				if (vpc->ch == ch)
					continue;

				found = TRUE;
				act(buf, ch, NULL, vpc->ch, TO_VICT);
				interpret(vpc->ch, argument);
			}

			if (!found)
				stc("You found no one to force.\n\r", ch);

			return;
		}
		else if (!str_cmp(arg, "players"))
		{
			for (vpc = pc_list; vpc != NULL; vpc = vpc_next)
			{
				vpc_next = vpc;

				if (vpc->ch != ch && !IS_IMMORTAL(vpc->ch))
				{
					found = TRUE;
					act(buf, ch, NULL, vpc->ch, TO_VICT);
					interpret(vpc->ch, argument);
				}
			}

			if (!found)
				stc("You found no one to force.\n\r", ch);

			return;
		}
		else if (!str_cmp(arg, "gods"))
		{
			for (vpc = pc_list; vpc != NULL; vpc = vpc_next)
			{
				vpc_next = vpc;

				if (vpc->ch != ch && IS_IMMORTAL(vpc->ch) && !IS_IMP(vpc->ch))
				{
					found = TRUE;
					vpc_next = vpc->next;
					act(buf, ch, NULL, vpc->ch, TO_VICT);
					interpret(vpc->ch, argument);
				}
			}

			if (!found)
				stc("You found no one to force.\n\r", ch);

			return;
		}
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		stc("Try forcing yourself to do your homework buddy!\n\r", ch);
		return;
	}

	if (!is_room_owner(ch, victim->in_room)
	 && ch->in_room != victim->in_room
	 && room_is_private(victim->in_room)
	 && !IS_IMP(ch))
	{
		stc("That character is in a private room.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim)
	 && !IS_IMP(ch)
	 && !IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)
	 && !IS_SET(victim->pcdata->plr, PLR_SNOOP_PROOF))
	{
		stc("Not at your level!\n\r", ch);
		return;
	}

	act(buf, ch, NULL, victim, TO_VICT);
	interpret(victim, argument);
	stc("You succeed.\n\r", ch);
}


void do_freeze(CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	CHAR_DATA *victim;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  freeze <player>\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		stc("Now that's just silly.\n\r", ch);
		return;
	}

	if ((IS_IMMORTAL(victim) && !IS_IMP(ch)) || IS_IMP(victim))
	{
		stc("You shrink in comparison to the power of your victim.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_FREEZE))
	{
		REMOVE_BIT(victim->act, PLR_FREEZE);
		stc("Heat envelops your blood.\n\r", victim);
		stc("FREEZE removed.\n\r", ch);
		sprintf(buf, "$N has unfrozen: %s.", victim->name);
		wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT(victim->act, PLR_FREEZE);
		stc("A crystal blue sheet of ice immobilizes your body!\n\r", victim);
		stc("FREEZE set.\n\r", ch);
		sprintf(buf, "$N puts %s in the deep freeze.", victim->name);
		wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

	save_char_obj(victim);
}


void do_fry(CHAR_DATA *ch, char *argument)
{
	char strsave[MIL], arg[MIL];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	/* prevent Alara from accidentally frying people :) -- Montrey */
	if (arg[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  fry <player>\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim))
	{
		stc("That's not a good idea.\n\r", ch);
		return;
	}

	act("You {Pdestroy{x $N and remove $M from Legacy!", ch, NULL, victim, TO_CHAR);
	act("A huge bolt of {Wlightning{x strikes $N, utterly {Pdestroying{x $M.", ch, NULL, victim, TO_NOTVICT);
	act("You look up, just in time to see the {Pflaming{x {Wlightning{x bolt strike your head. C-ya!", ch, NULL, victim, TO_VICT);
	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
	do_echo(ch, "You hear the rumble of thunder in the distance.");
	update_pc_index(victim, TRUE);
	do_fuckoff(victim, "");
	unlink(strsave);
}


void do_locker(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  locker <victim> <get|put> <object>\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL)
	{
		stc("They must be online to examine their locker.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		ptc(ch, "%s's locker contains:\n\r", victim->name);
		show_list_to_char(victim->pcdata->locker, ch, TRUE, TRUE, TRUE);
		return;
	}

	argument = one_argument(argument, arg2);

	if (arg2[0] == '\0' || argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  locker <player> <get|put> <object>\n\r", ch);
		return;
	}

	if (!str_prefix1(arg2, "get"))
	{
		if ((obj = get_obj_list(ch, argument, victim->pcdata->locker)) == NULL)
		{
			stc("You do not see that in their locker.\n\r", ch);
			return;
		}

		obj_from_locker(obj);
		obj_to_char(obj, ch);
		ptc(ch, "You get %s from %s's locker.\n\r", obj->short_descr, victim->name);
		return;
        }

	if (!str_prefix1(arg2, "put"))
	{
		if ((obj = get_obj_carry(ch, argument)) == NULL)
		{
			stc("You do not have that item.\n\r", ch);
			return;
		}

		obj_from_char(obj);
		obj_to_locker(obj, victim);
		ptc(ch, "You put %s in %s's locker.\n\r", obj->short_descr, victim->name);
		return;
	}

	stc("Syntax:\n\r"
	    "  locker <player> <get|put> <object>\n\r", ch);
}


void do_strongbox(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  strongbox <victim> <get|put> <object>\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL)
	{
		stc("They must be online to examine their strongbox.\n\r", ch);
		return;
	}

	if (!IS_HEROIC(victim))
	{
		stc("Only heroes and remorts have strongboxes.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		ptc(ch, "%s's strongbox contains:\n\r", victim->name);
		show_list_to_char(victim->pcdata->strongbox, ch, TRUE, TRUE, TRUE);
		return;
	}

	argument = one_argument(argument, arg2);

	if (arg2[0] == '\0' || argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  strongbox <player> <get|put> <object>\n\r", ch);
		return;
	}

	if (!str_prefix1(arg2, "get"))
	{
		if ((obj = get_obj_list(ch, argument, victim->pcdata->strongbox)) == NULL)
		{
			stc("You do not see that in their strongbox.\n\r", ch);
			return;
		}

		obj_from_strongbox(obj);
		obj_to_char(obj, ch);
		ptc(ch, "You get %s from %s's strongbox.\n\r", obj->short_descr, victim->name);
		return;
        }

	if (!str_prefix1(arg2, "put"))
	{
		if ((obj = get_obj_carry(ch, argument)) == NULL)
		{
			stc("You do not have that item.\n\r", ch);
			return;
		}

		obj_from_char(obj);
		obj_to_strongbox(obj, victim);
		ptc(ch, "You put %s in %s's strongbox.\n\r", obj->short_descr, victim->name);
		return;
	}

	stc("Syntax:\n\r"
	    "  strongbox <player> <get|put> <object>\n\r", ch);
}


void do_log(CHAR_DATA *ch, char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  log <player>\n\r"
		    "  log all\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (!str_cmp(arg, "all"))
	{
		fLogAll = !fLogAll;
		ptc(ch, "Log ALL %s.\n\r", fLogAll ? "on" : "off");
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_LOG))
		REMOVE_BIT(victim->act, PLR_LOG);
	else
		SET_BIT(victim->act, PLR_LOG);

	ptc(ch, "LOG %s.\n\r", IS_SET(victim->act, PLR_LOG) ? "set" : "removed");
}


void do_newlock(CHAR_DATA *ch, char *argument)
{
	extern bool newlock;

	newlock = !newlock;

	if (newlock)
	{
		wiznet("$N locks out new characters.", ch, NULL, 0, 0, 0);
		stc("New characters have been locked out.\n\r", ch);
	}
	else
	{
		wiznet("$N allows new characters back in.", ch, NULL, 0, 0, 0);
		stc("Newlock removed.\n\r", ch);
	}
}


void do_newpasswd(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL];
	CHAR_DATA *victim;
	char *pwdnew, *p;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  newpasswd <player> <passwd>\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMP(ch))
	{
		stc("You are not high enough level to change their password.\n\r", ch);
		return;
	}

	if (strlen(argument) < 5)
	{
		stc("Password must be at least five characters long.\n\r", ch);
		return;
	}

        pwdnew = str_dup(argument);

	for (p = pwdnew; *p != '\0'; p++)
	{
		if (*p == '~')
		{
			stc("New password not acceptable, try again.\n\r", ch);
			return;
		}
	}

	free_string(victim->pcdata->pwd);
	victim->pcdata->pwd = str_dup(pwdnew);
	free_string(pwdnew);
	ptc(ch, "%s's new password is: %s\n\r", victim->name, argument);
	save_char_obj(victim);
	sprintf(buf, "$N has changed %s's password.", victim->name);
	wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, 0);
}


void do_pardon(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  pardon <player> <killer|thief>\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (!str_cmp(arg2, "killer"))
	{
		if (IS_SET(victim->act, PLR_KILLER))
		{
			REMOVE_BIT(victim->act, PLR_KILLER);
			stc("Their killer flag has been removed.\n\r", ch);
			stc("You are no longer a KILLER.\n\r", victim);
			REMOVE_BIT(victim->act, PLR_NOPK);
		}
		else
			stc("They do not have a killer flag set.\n\r", ch);

		ch->pcdata->flag_killer = 0;
	}
	else if (!str_cmp(arg2, "thief"))
	{
		if (IS_SET(victim->act, PLR_THIEF))
		{
			REMOVE_BIT(victim->act, PLR_THIEF);
			stc("Their thief flag has been removed.\n\r", ch);
			stc("You are no longer a THIEF.\n\r", victim);
			REMOVE_BIT(victim->act, PLR_NOPK);
		}
		else
			stc("They do not have a thief flag set.\n\r", ch);

		ch->pcdata->flag_thief = 0;
	}
	else
		stc("Syntax:\n\r"
		    "  pardon <player> <killer|thief>\n\r", ch);
}


void do_protect(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  protect <player>\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL)
	{
		stc("You can't find them.\n\r", ch);
		return;
	}

	if (IS_SET(victim->pcdata->plr, PLR_SNOOP_PROOF))
	{
		ptc(ch, "%s is no longer protected from snooping.\n\r", victim->name);
		REMOVE_BIT(victim->pcdata->plr, PLR_SNOOP_PROOF);
	}
	else
	{
		ptc(ch, "You protect %s from snooping.\n\r", victim->name);
		SET_BIT(victim->pcdata->plr, PLR_SNOOP_PROOF);
	}
}


void do_revoke(CHAR_DATA *ch, char *argument)
{
	char buf1[MSL], buf2[MSL], arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;
	int i;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		long printed = 0;

		/* print a list of revokable stuff */
		stc("Current REVOKE options:\n\r\n\r", ch);

		for (i = 0; revoke_table[i].name != NULL; i++)
		{
			/* don't print the same one twice :) */
			if (IS_SET(printed, revoke_table[i].bit))
				continue;

			ptc(ch, "  %s\n\r", revoke_table[i].name);
			SET_BIT(printed, revoke_table[i].bit);
		}

		stc("\n\rSyntax:\n\r"
		    "  revoke <character> <option>\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL)
	{
		stc("They aren't here.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMP(ch))
	{
		stc("You have failed.\n\r", ch);
		return;
	}

	for (i = 0; revoke_table[i].name != NULL; i++)
	{
		if (str_prefix1(arg2, revoke_table[i].name))
			continue;

		if (IS_SET(victim->revoke, revoke_table[i].bit))
		{
			REMOVE_BIT(victim->revoke, revoke_table[i].bit);
			sprintf(buf1, "restore");
		}
		else
		{
			SET_BIT(victim->revoke, revoke_table[i].bit);
			sprintf(buf1, "revoke");
		}

		ptc(victim, "The Gods have %sd your %s.\n\r", buf1, revoke_table[i].message);
		ptc(ch, "You %s their %s.\n\r", buf1, revoke_table[i].message);
		sprintf(buf2, "$N has %sd %s's %s", buf1, victim->name, revoke_table[i].message);
		wiznet(buf2, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
		return;
	}

	stc("Revoke WHAT?\n\r", ch);
}


/* like snoop, but better -- Elrac */
int set_tail(CHAR_DATA *ch, CHAR_DATA *victim, int tail_flag)
{
	CHAR_DATA *wch;
	TAIL_DATA *td;

	/* if global untail, try specific untail on all chars having tail data */
	if (victim == NULL)
	{
		int count = 0;

		for (wch = char_list; wch; wch = wch->next)
			if (wch->tail)
				count += set_tail(ch, wch, TAIL_NONE);

		return count;
	}

	/* start tailing someone */
	if (tail_flag != TAIL_NONE)
	{
		/* find previous tail by same ch, if any */
		for (td = victim->tail; td; td = td->next)
			if (td->tailed_by == ch)
				break;

		/* if none, build and link a new tail data item */
		if (!td)
		{
			td = alloc_mem(sizeof(struct tail_data));
			td->tailed_by = ch;
			td->tailer_name = str_dup(ch->name);
			td->flags = 0;
			td->next = victim->tail;
			victim->tail = td;
		}

		td->flags |= tail_flag;
		ptc(ch, "You are now tailing %s:%s\n\r",
			PERS(victim, ch, VIS_PLR),
			(td->flags & TAIL_ACT) ? " ACT" : "");
		return 1;
	}

	/* specific untail */
	if (victim->tail == NULL)
		return 0;

	/* find tail data set by ch */
	for (td = victim->tail; td; td = td->next)
		if (td->tailed_by == ch)
			break;

	/* if snooped by ch, remove the tail item */
	if (td)
	{
		if (td != victim->tail)
		{
			/* not first in chain, so copy first over retiring item */
			VALIDATE(td);
			td->tailed_by = victim->tail->tailed_by;
			td->tailer_name = victim->tail->tailer_name;
			td->flags = victim->tail->flags;
			td = victim->tail;
		}

		/* lop off first item */
		victim->tail = td->next;
		free_string(td->tailer_name);
		INVALIDATE(td);
		free_mem(td, sizeof(struct tail_data));

		if (IS_VALID(ch))
			ptc(ch, "You have stopped tailing %s.\n\r", PERS(victim, ch, VIS_PLR));

		return 1;
	}

	return 0;
}


void do_tail(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL];
	CHAR_DATA *victim = NULL;

	if (IS_NPC(ch))
	{
		stc("Please return to your body before tailing.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  tail <victim> act\n\r"
		    "  tail <victim> stop\n\r"
		    "  tail stop\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (!str_cmp(arg, "stop"))
	{
		if (!set_tail(ch, NULL, TAIL_NONE))
			stc("You weren't tailing anyone.\n\r", ch);

		ch->pcdata->tailing = FALSE;
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL)
	{
		stc("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		stc("Please don't try to tail yourself -- you'll get dizzy!\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (!arg[0])
	{
		do_tail(ch, "");
		return;
	}

	if (!str_prefix1(arg, "stop"))
	{
		if (!set_tail(ch, victim, TAIL_NONE))
			stc("You were not tailing them.\n\r", ch);
	}
	else if (!str_prefix1(arg, "actions"))
	{
		set_tail(ch, victim, TAIL_ACT);
		ch->pcdata->tailing = TRUE;
		sprintf(buf, "$N has begun tailing %s.", PERS(victim, ch, VIS_PLR));
		wiznet(buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, GET_RANK(ch));
	}
	else
		do_tail(ch, "");
}


void do_snoop(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  snoop <player>\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL)
	{
		stc("They aren't here.\n\r", ch);
		return;
	}

	if (victim->desc == NULL)
	{
		stc("No descriptor to snoop.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		stc("All snoops in progress have been cancelled.\n\r", ch);
		wiznet("$N has cancelled all snoops.", ch, NULL, WIZ_SNOOPS, WIZ_SECURE, GET_RANK(ch));

		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;

		return;
	}

	if (victim->desc->snoop_by != NULL)
	{
		stc("That character is already being snooped.\n\r", ch);
		return;
	}

	if (!is_room_owner(ch, victim->in_room)
	 && ch->in_room != victim->in_room
	 && room_is_private(victim->in_room)
	 && !IS_IMP(ch))
	{
		stc("That character is in a private room.\n\r", ch);
		return;
	}

	if ((IS_IMMORTAL(victim) || IS_SET(victim->pcdata->plr, PLR_SNOOP_PROOF)) && !IS_IMP(ch))
	{
		stc("They wouldn't like that!\n\r", ch);
		return;
	}

        if ( (! strcasecmp(victim->name, "teotwawki") ) ||
             (! strcasecmp(victim->name, "outsider") ) )
        {
            stc("They would not like that.\n\r", ch);
            return;
        }


	if (ch->desc != NULL)
	{
		for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
		{
			if (d->character == victim || d->original == victim)
			{
				stc("Sorry, we don't allow snoop loops.\n\r", ch);
				return;
			}
		}
	}

	victim->desc->snoop_by = ch->desc;
	sprintf(buf,"$N has begun a snoop on %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, GET_RANK(ch));
	stc("You now view the world through the players eyes.\n\r", ch);
}


void do_ban(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL], site[MIL];
	char *p;
	int flags = 0;
	MYSQL_RES *result;

	if (argument[0] == '\0')
	{
		MYSQL_ROW row;
		BUFFER *output;
		bool found = FALSE;

		if ((result = db_query("do_ban", "SELECT site, name, flags, reason FROM bans")) == NULL)
			return;

		output = new_buf();
		add_buf(output, "Banned Sites                  {T|{xBanned by      {T|{xType   {T|{xReason\n\r");
		add_buf(output, "{T------------------------------+---------------+-------+-----------------------------{x\n\r");

		while ((row = mysql_fetch_row(result)))
		{
			found = TRUE;
			flags = atoi(row[2]);
			sprintf(site, "%s%s%s",
				IS_SET(flags, BAN_PREFIX) ? "*" : "",
				row[0],
				IS_SET(flags, BAN_SUFFIX) ? "*" : "");

			ptb(output, "%-30s{T|{x%-15s{T|{x%s{T|{x%s\n\r",
				site,
				row[1],
				IS_SET(flags, BAN_PERMIT)  ? "PERMIT " :
				IS_SET(flags, BAN_ALL)     ? "  ALL  " :
				IS_SET(flags, BAN_NEWBIES) ? "NEWBIES" : "       ",
				row[3]);
		}

		if (found)
			page_to_char(buf_string(output), ch);
		else
			stc("There are no banned hosts.\n\r", ch);

		free_buf(output);
		mysql_free_result(result);
		return;
	}

	/* arg1 needs to not have apostrophes, as does argument after all this.
	   reason is that we need to sprintf both arg1 and argument into the
	   query, and a char function using a static char can't do it twice in
	   the same sprintf */
	argument = one_argument(db_esc(argument), arg1);
	argument = one_argument(argument, arg2);

	if (strlen(arg1) < 5 || strlen(arg1) > 30)
	{
		stc("The site must be between 5 and 30 characters long.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0')
	{
		stc("You must specify a ban type.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		stc("You must provide a reason.\n\r", ch);
		return;
	}

	if (strlen(argument) > 45)
	{
		stc("Please limit the reason to 45 characters or less.\n\r", ch);
		return;
	}

	     if (!str_cmp(arg2, "all"))		SET_BIT(flags, BAN_ALL);
	else if (!str_cmp(arg2, "newbies"))	SET_BIT(flags, BAN_NEWBIES);
	else
	{
		stc("Type must be ALL or NEWBIES.\n\r", ch);
		return;
	}

	p = arg1;

	if (*p == '*')
	{
		sprintf(site, p+1);
		SET_BIT(flags, BAN_PREFIX);
	}
	else
		sprintf(site, p);

	for (p = site; *(p+1) != '\0'; p++);

	if (*p == '*')
	{
		*p = '\0';
		SET_BIT(flags, BAN_SUFFIX);
	}

	if (db_countf("do_ban", "SELECT COUNT(*) FROM bans WHERE site='%s' AND (flags-((flags>>4)<<4))=%d",
			site, flags) > 0)
		ptc(ch, "%s is already banned.\n\r", arg1);
	else
	{
		db_commandf("do_ban", "INSERT INTO bans VALUES('%s','%s',%d,'%s')",
			db_esc(site), db_esc(ch->name), flags, db_esc(argument));
		ptc(ch, "%s has been banned.\n\r", arg1);
	}
}


void do_allow(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], site[MIL];
	char *p;
	int wildflags = 0;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		stc("Remove which site from the ban list?\n\r", ch);
		return;
	}

	if (strlen(arg) < 5)
	{
		stc("Banned sites are no shorter than five characters.\n\r", ch);
		return;
	}

	p = arg;

	if (*p == '*')
	{
		sprintf(site, p+1);
		SET_BIT(wildflags, BAN_PREFIX);
	}
	else
		sprintf(site, p);

	for (p = site; *(p+1) != '\0'; p++);

	if (*p == '*')
	{
		*p = '\0';
		SET_BIT(wildflags, BAN_SUFFIX);
	}

	db_commandf("do_allow", "DELETE FROM bans WHERE site='%s' AND (flags-((flags>>2)<<2))=%d",
		db_esc(site), wildflags);

	if (mysql_affected_rows(mysql_db))
		ptc(ch, "Ban on %s lifted.\n\r", arg);
	else
		stc("That site is not banned.\n\r", ch);
}


void do_permit(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], site[MIL];
	char *p;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int wildflags = 0;
	bool found = FALSE;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  permit <site>\n\r"
		    "  permit player <name>\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "player"))
	{
		CHAR_DATA *plr;

		if ((plr = get_player_world(ch, arg, VIS_PLR)) == NULL)
		{
			stc("They are not playing or loaded.\n\r", ch);
			return;
		}

		if (GET_RANK(ch) < GET_RANK(plr))
		{
			stc("They are beyond your powers.\n\r", ch);
			return;
		}

		if (IS_SET(plr->act, PLR_PERMIT))
		{
			stc("They are no longer permitted to bypass banned sites.\n\r", ch);
			REMOVE_BIT(plr->act, PLR_PERMIT);
		}
		else
		{
			stc("They are now permitted to bypass banned sites.\n\r", ch);
			SET_BIT(plr->act, PLR_PERMIT);
		}

		return;
	}

	if (strlen(arg) < 5)
	{
		stc("Banned sites are no shorter than five characters.\n\r", ch);
		return;
	}

	p = arg;

	if (*p == '*')
	{
		sprintf(site, p+1);
		SET_BIT(wildflags, BAN_PREFIX);
	}
	else
		sprintf(site, p);

	for (p = site; *(p+1) != '\0'; p++);

	if (*p == '*')
	{
		*p = '\0';
		SET_BIT(wildflags, BAN_SUFFIX);
	}

	if ((result = db_queryf("do_permit",
			"SELECT flags, site FROM bans WHERE site='%s' AND (flags-((flags>>2)<<2))=%d",
			db_esc(site), wildflags)) == NULL)
		return;

	while (!found && (row = mysql_fetch_row(result)))
	{
		int rowflags = atoi(row[0]);

		if (IS_SET(rowflags, BAN_PERMIT))
		{
			ptc(ch, "Permit flag removed on %s.\n\r", arg);
			REMOVE_BIT(rowflags, BAN_PERMIT);
		}
		else
		{
			ptc(ch, "Permit flag set on %s.\n\r", arg);
			SET_BIT(rowflags, BAN_PERMIT);
		}

		db_commandf("do_permit", "UPDATE bans SET flags=%d WHERE site='%s' AND flags=%d",
			rowflags, db_esc(row[1]), atoi(row[0]));

		found = TRUE;
	}

	if (!found)
		stc("That site is not banned.\n\r", ch);

	mysql_free_result(result);
}


void do_deny(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg1[MIL];

	if (argument[0] == '\0')
	{
		MYSQL_RES *result;
		MYSQL_ROW row;
		BUFFER *output;
		bool found = FALSE;

		if ((result = db_query("do_deny", "SELECT name, denier, reason FROM denies")) == NULL)
			return;

		output = new_buf();
		add_buf(output, "Denied Players {T|{xDenied by      {T|{xReason\n\r");
		add_buf(output, "{T---------------+---------------+-------------------------------------------{x\n\r");

		while ((row = mysql_fetch_row(result)))
		{
			found = TRUE;
			ptb(output, "%-15s{T|{x%-15s{T|{x%s\n\r",
				row[0],
				row[1],
				row[2]);
		}

		if (found)
			page_to_char(buf_string(output), ch);
		else
			stc("There are no denied players.\n\r", ch);

		free_buf(output);
		mysql_free_result(result);
		return;
	}

	argument = one_argument(db_esc(argument), arg1);

	if (argument[0] == '\0')
	{
		stc("You must provide a reason.\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL)
	{
		stc("They are not playing.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		stc("But, but, we need you!!\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMP(ch))
	{
		stc("You have failed.\n\r", ch);
		return;
	}

	if (db_countf("do_deny", "SELECT COUNT(*) FROM denies WHERE name='%s'", victim->name) > 0) 
		ptc(ch, "%s is already denied.\n\r", victim->name);
	else
	{
		char buf[MSL];

		db_commandf("do_deny", "INSERT INTO denies VALUES('%s','%s','%s')",
			db_esc(victim->name), db_esc(ch->name), db_esc(argument));

		stc("You have been denied access!\n\r", victim);
		sprintf(buf,"$N has denied access to %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		ptc(ch, "%s has been denied access.\n\r", arg1);
		save_char_obj(victim);
		stop_fighting(victim,TRUE);
		do_quit(victim, "now");
	}
}


void do_undeny(CHAR_DATA *ch, char *argument)
{
	char arg[MIL];

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		stc("Undeny which player?\n\r", ch);
		return;
	}

	db_commandf("do_undeny", "DELETE FROM denies WHERE name='%s'", db_esc(arg));

	if (mysql_affected_rows(mysql_db))
	{
		char buf[MSL];

		ptc(ch, "%s has been granted access to Legacy.\n\r", arg);
		sprintf(buf, "$N has undenied %s", arg);
		wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
		stc("That player is not denied.\n\r", ch);
}

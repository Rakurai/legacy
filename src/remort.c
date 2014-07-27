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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "magic.h"

/*****
 Remort Affects Stuff
 *****/

/* take a raff id and return the index number, 0 if a null raff */
int raff_lookup ( int index )
{
    int i;

    for (i = 1; i < MAX_RAFFECTS; i++)
    {
	if (raffects[i].id == index)
	    return i;
	/* returns 0 on the first raff, which is null anyway */
    }

    return 0;
}

void fix_blank_raff ( CHAR_DATA *ch, int start )
{
    /* hunts through a remort's raffects, takes out zeroes except at the end */
    /* start is the starting point, most cases use 0 */
    int i, x, last;

    if (IS_NPC(ch) || !IS_REMORT(ch))
	return;

    last = ((ch->pcdata->remort_count / 10) + 1);

    /* loop through them all starting where told to */
    for (i = start; i < last; i++)
    {
	/* if it's null, fix it by shifting all raffects down one */
	if (ch->pcdata->raffect[i] < 1)
	{
	    for (x = i; x < last; x++)
		/* set the affect equal to the next one, if it's the last, set to 0 in case they have more */
		ch->pcdata->raffect[x] = ((x == last) ? 0 : ch->pcdata->raffect[x+1]);
					/* max raffect fields is 10, set last one to 0 */
	}
    }
}

void rem_raff_affect ( CHAR_DATA *ch, int index )
{
    if (raffects[index].add)
    {
	if ((raffects[index].id >= 900) && (raffects[index].id <= 949))
	    REMOVE_BIT(ch->vuln_flags, raffects[index].add);
	else if ((raffects[index].id >= 950) && (raffects[index].id <= 999))
	    REMOVE_BIT(ch->res_flags, raffects[index].add);
    }
    return;
}

bool HAS_RAFF( CHAR_DATA *ch, int flag )
{
    int i;

    if (flag <= 0)
	return FALSE;

    if (IS_NPC(ch))
    	return FALSE;

    for (i = 0; i < ((ch->pcdata->remort_count / 10) + 1); i++)
        {
            if (ch->pcdata->raffect[i] == flag)
            	return TRUE;
        }

    return FALSE;
}

bool HAS_RAFF_GROUP( CHAR_DATA *ch, int flag )
{
    int i;
 
    if (IS_NPC(ch))
        return FALSE;
    
    for (i = 0; i < ((ch->pcdata->remort_count / 10) + 1); i++)
        {
            if (raffects[raff_lookup(ch->pcdata->raffect[i])].group == flag)
                return TRUE;
        }
    
    return FALSE;
}

void roll_one_raff ( CHAR_DATA *ch, CHAR_DATA *victim, int place )
{
    int test;
    bool can_add = FALSE;

    do
    {
	test = number_range(1,MAX_RAFFECTS);

	while (test < 1 || test > MAX_RAFFECTS
	 || raffects[test].id < 1
	 || number_percent() < raffects[test].chance)
	    test = number_range(1,MAX_RAFFECTS);

	/* Check percentage chance, and whether it's a 'good' or 'bad' remort affect */
	if ((raffects[test].id >= 1)
	 && (raffects[test].id <= 99) /* if it's a good one... */
	 && (number_percent() <= (raffects[test].chance + (victim->pcdata->remort_count / 10))))
	    can_add = TRUE;
	else if ((raffects[test].id >= 100)
	 && (raffects[test].id <= 199) /* if it's a bad one... */
	 && (number_percent() <= (raffects[test].chance - (victim->pcdata->remort_count / 10))))
	    can_add = TRUE;
	else if ((raffects[test].id >= 900)
	 && (raffects[test].id <= 949) /* if it's a vuln... */
	 && (number_percent() <= (raffects[test].chance - (victim->pcdata->remort_count / 10)))
	 && (!IS_SET(victim->vuln_flags,raffects[test].add)) /* checks current vulns */
	 && (!IS_SET(victim->res_flags,raffects[test].add))) /* checks for opposite */
	    can_add = TRUE;
	else if ((raffects[test].id >= 950)
	 && (raffects[test].id <= 999) /* if it's a res... */
	 && (number_percent() <= (raffects[test].chance + (victim->pcdata->remort_count / 10)))
	 && (!IS_SET(victim->res_flags,raffects[test].add)) /* checks current res's */
	 && (!IS_SET(victim->vuln_flags,raffects[test].add))) /* checks for opposite */
	    can_add = TRUE;

	if (HAS_RAFF(victim,raffects[test].id))
	    can_add = FALSE;

	/* check if the group isn't 0, and then set can_add to FALSE if they have the group */
	if (raffects[test].group != 0)
	    if (HAS_RAFF_GROUP(victim,raffects[test].group))
		can_add = FALSE;
    }
    while (!can_add);

    victim->pcdata->raffect[place] = raffects[test].id;

    if (raffects[test].add != 0)
    {
	if ((raffects[test].id >= 900) && (raffects[test].id <= 949))
	    SET_BIT(victim->vuln_flags,raffects[test].add);
	else if ((raffects[test].id >= 950) && (raffects[test].id <= 999))
	    SET_BIT(victim->res_flags,raffects[test].add);
    }

    if (ch != victim)
	ptc(ch,"({C%3d{x) {W%s{x added.\n\r",raffects[test].id,str_dup(raffects[test].description));
    ptc(victim,"{C--- {W%s.{x\n\r",str_dup(raffects[test].description));
}

void roll_raffects ( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* not putting a lot of condition checking in this because it's only used in set and remort, we'll
   assume that the ch and victim are pcs, ch is an imm, victim is a remort */

    int c;

    for( c = 0; c < victim->pcdata->remort_count / 10+1; c++)
	roll_one_raff(ch,victim,c);
}


/*****
 Extraclass Stuff
 *****/

bool HAS_EXTRACLASS( CHAR_DATA *ch, int sn )
{
    int i;

    if (sn <= 0)
	return FALSE;

    if (IS_NPC(ch))
    	return FALSE;

    for (i = 0; i < ((ch->pcdata->remort_count / 20) + 1); i++)
        {
            if (ch->pcdata->extraclass[i] == sn)
            	return TRUE;
        }

    return FALSE;
}

bool CAN_USE_RSKILL( CHAR_DATA *ch, int sn )
{
	if (IS_NPC(ch))
	{
		if (skill_table[sn].spell_fun == spell_null)
			return FALSE;

		return TRUE;
	}

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (!IS_REMORT(ch))
		return FALSE;

	if (!get_skill(ch,sn))
		return FALSE;

	if ((ch->class+1 != skill_table[sn].remort_class) && (!HAS_EXTRACLASS(ch,sn)))
		return FALSE;

	return TRUE;
}


void list_extraskill( CHAR_DATA *ch )
{
	BUFFER *output;
	int sn, cn, col;

	output = new_buf();
	add_buf(output, "\n\r                      {BExtraclass Remort Skills{x\n\r");

	for (cn = 0; cn < MAX_CLASS; cn++)
	{
		if (!IS_IMMORTAL(ch))
			if (cn == ch->class)
				continue;

		ptb(output, "\n\r{W%s Skills{x\n\r    ", capitalize(class_table[cn].name));

		for (sn = 0, col = 0; skill_table[sn].name != NULL; sn++)
		{
			if (skill_table[sn].remort_class != cn+1)
				continue;

			if (!IS_IMMORTAL(ch)
			 && (skill_table[sn].remort_class == ch->class+1
			  || skill_table[sn].skill_level[ch->class] <= 0
			  || skill_table[sn].skill_level[ch->class] > 91))
				continue;

			ptb(output, "%-15s %s%-8d{x",
				skill_table[sn].name,
				ch->train >= skill_table[sn].rating[ch->class] ? "{C" : "{T",
				skill_table[sn].rating[ch->class]);

			if (++col % 3 == 0)
				add_buf(output, "\n\r");
		}

		add_buf(output, "\n\r");

		if (col % 3 != 0)
			add_buf(output,"\n\r");
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
}


void do_eremort( CHAR_DATA *ch, char *argument )
{
	char arg1[MIL];
	BUFFER *output;
	int x, sn = 0;

	argument = one_argument(argument, arg1);

	if (IS_NPC(ch))
	{
		do_huh(ch);
		return;
	}

	if (IS_IMMORTAL(ch)) /* just list em for imms */
	{
		list_extraskill(ch);
		return;
	}

	if (ch->pcdata->remort_count < 1)
	{
		do_huh(ch);
		return;
	}

	output = new_buf();

	if (arg1[0] == '\0')
	{
		list_extraskill(ch);

		if (ch->pcdata->extraclass[0] +
		    ch->pcdata->extraclass[1] +
		    ch->pcdata->extraclass[2] +
		    ch->pcdata->extraclass[3] +
		    ch->pcdata->extraclass[4] > 0)
		{
			ptb(output, "\n\rYour current extraclass skill%s",
				ch->pcdata->extraclass[1] ? "s are" : " is");

			if (ch->pcdata->extraclass[0])
				ptb(output, " %s",
					skill_table[ch->pcdata->extraclass[0]].name);

			for (x = 1; x < ch->pcdata->remort_count / 20 + 1; x++)
				if (ch->pcdata->extraclass[x])
					ptb(output, ", %s",
						skill_table[ch->pcdata->extraclass[x]].name);

			add_buf(output,".\n\r");
			page_to_char(buf_string(output), ch);
			free_buf(output);
		}

		return;
	}

	/* Ok, now we check to see if the skill is a remort skill */
	if ((sn = skill_lookup(arg1)) < 0)
	{
		stc("That is not even a valid skill, much less a remort skill.\n\r", ch);
		return;
	}

	/* Is it a remort skill? */
	if (skill_table[sn].remort_class == 0)
	{
		stc("That is not a remort skill.\n\r", ch);
		return;
	}

	/* Is it outside of the player's class? */
	if (skill_table[sn].remort_class == ch->class+1)
	{
		stc("You have knowledge of this skill already, pick one outside your class.\n\r", ch);
		return;
	}

	/* is it barred from that class? */
	if (skill_table[sn].skill_level[ch->class] <= 0
	 || skill_table[sn].skill_level[ch->class] > 91)
	{
		stc("Your class cannot gain that skill.\n\r", ch);
		return;
	}

	/* do they have it already? */
	if (HAS_EXTRACLASS(ch, sn))
	{
		stc("You already know that skill.\n\r", ch);
		return;
	}

	if (ch->train < skill_table[sn].rating[ch->class])
	{
		stc("You do not have enough training to master this skill.\n\r", ch);
		return;
	}

	/* find the first blank spot, and add the skill */
	for (x = 0; x < ch->pcdata->remort_count / 20 + 1; x++)
	{
		if (!ch->pcdata->extraclass[x])
		{
			ch->pcdata->extraclass[x] = sn;

			if (!ch->pcdata->learned[sn])
				ch->pcdata->learned[sn] = 1;

			ch->train -= skill_table[sn].rating[ch->class];
			ptc(ch, "You have gained %s as an extraclass remort skill.\n\r",
				skill_table[sn].name);
			return;
		}
	}

	/* can't find an empty spot, must have the max number of extraclass skills */
	stc("You have enough extraclass remort skills!  Don't be greedy!\n\r", ch);
}


/* Remort Code by Lotus */
/* Redone by Endo and Montrey */
void do_remort( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA *af, *af_next;
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[MSL];
	int race, x, c;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0'
	 || (arg2[0] != '\0'
	  && (arg3[0] == '\0'
	   || argument[0] == '\0')))
	{
		stc("Syntax:\n\r"
		    "  remort <victim>   (must be remort 1 or higher)\n\r"
		    "  remort <victim> <race> <deity> <title>\n\r", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL)
	{
		stc("Hmmm...they must have ran off in fear >=).\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim))
	{
		stc("This can only be used on mortals.\n\r",ch);
		return;
	}

	if (victim->level != LEVEL_HERO)
	{
		stc("Only heroes can remort.\n\r", ch);
		return;
	}

	if (victim->pcdata->remort_count >= 99) /* To keep who in line -Endo */
	{
		stc("That player can no longer remort.  If you've hit this limit you need\n\r"
		    "to talk the coders into changing the who list.  Aren't you special?\n\r", ch);
		return;
	}

	if (arg2[0] == '\0' && victim->pcdata->remort_count < 1)
	{
		stc("Syntax:\n\r"
		    "  remort <victim> <race> <deity> <title>\n\r", ch);
		return;
	}

	/* they gotta be naked */
	for (x = 0; x < MAX_WEAR; x++)
	{
		if (get_eq_char(victim, x) != NULL)
		{
			stc("Tell them to remove all of their eq first.\n\r", ch);
			return;
		}
	}

	if (arg2[0] == '\0')
		race = victim->race;
	else
	{
		race = race_lookup(arg2);

		if ((race = race_lookup(arg2)) == 0
		 || !race_table[race].pc_race )
		{
			stc("That is not a valid race.  Please choose from:\n\r", ch);

			for (race = 1; race_table[race].name != NULL && race_table[race].pc_race; race++)
			{
				stc(race_table[race].name, ch);
				stc(" ", ch);
			}

			stc("\n\r", ch);
			return;
		}

		if ((victim->pcdata->remort_count + 1) < pc_race_table[race].remort_level)
		{
			stc("They are not experienced enough for that race.\n\r", ch);
			return;
		}
	}

	for (af = victim->affected; af != NULL; af = af_next)
	{
		af_next = af->next;
		affect_remove(victim, af);
	}

	victim->level			= 1;
	victim->race			= race;
	victim->max_hit			= 20;
	victim->max_mana		= 100;
	victim->max_stam		= 100;
	victim->hit			= victim->max_hit;
	victim->mana			= victim->max_mana;
	victim->stam			= victim->max_stam;
	victim->pcdata->perm_hit	= victim->max_hit;
	victim->pcdata->perm_mana	= victim->max_mana;
	victim->pcdata->perm_stam	= victim->max_stam;
	victim->affected_by		= race_table[race].aff;
	victim->imm_flags		= race_table[race].imm;
	victim->res_flags		= race_table[race].res;
	victim->vuln_flags		= race_table[race].vuln;
	victim->form			= race_table[race].form;
	victim->parts			= race_table[race].parts;

	/* make sure stats aren't above their new maximums */
	for (x = 0; x < MAX_STATS; x++)
		if (victim->perm_stat[x] > get_max_train(victim, x))
			victim->perm_stat[x] = get_max_train(victim, x);

	if (arg2[0] != '\0')
	{
		free_string(victim->pcdata->deity);
		free_string(victim->pcdata->status);
		victim->pcdata->deity = str_dup(arg3);
		victim->pcdata->status = str_dup(argument);
	}

	victim->pcdata->remort_count++;
	victim->exp = exp_per_level(victim, victim->pcdata->points);

	if (victim->pet != NULL)
	{
		/* About the same stats as a Kitten */
		victim->pet->level			= 1;
		victim->pet->max_hit			= 20;
		victim->pet->max_mana			= 100;
		victim->pet->max_stam			= 100;
		victim->pet->hit			= victim->max_hit;
		victim->pet->mana			= victim->max_mana;
		victim->pet->stam			= victim->max_stam;
		victim->pet->hitroll			= 2;
		victim->pet->damroll			= 0;
		victim->pet->damage[DICE_NUMBER]	= 1;
		victim->pet->damage[DICE_TYPE]		= 4;
		victim->pet->perm_stat[STAT_STR]	= 12;
		victim->pet->perm_stat[STAT_INT]	= 12;
		victim->pet->perm_stat[STAT_WIS]	= 12;
		victim->pet->perm_stat[STAT_DEX]	= 12;
		victim->pet->perm_stat[STAT_CON]	= 12;
		victim->pet->perm_stat[STAT_CHR]	= 12;
		victim->pet->saving_throw		= 0;

		for (c = 0; c < 4; c++)
			victim->pet->armor_a[c] = 100;
	}

	victim->pcdata->trains_to_hit = 0;
	victim->pcdata->trains_to_mana = 0;
	victim->pcdata->trains_to_stam = 0;

	/* clear all old raffects */
	for (c = 0; c < 10; c++)
		victim->pcdata->raffect[c] = 0;

	/* clear all old extraclass skills */
	for (c = 0; c < 5; c++)
		victim->pcdata->extraclass[c] = 0;

	stc("Your deity bestows upon you...\n\r", victim);
	roll_raffects(ch, victim);

	if (victim->pcdata->remort_count == 1)
		SET_BIT(victim->pcdata->plr, PLR_SHOWRAFF);

	sprintf(buf, "%s has been reborn!", victim->name );
	do_send_announce(victim, buf);
	stc("You suddenly feel like a newbie!! Do'h!!!\n\r", victim);
	stc("Successful Remort.\n\r", ch);
}


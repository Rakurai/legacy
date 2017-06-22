#pragma once
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include "declare.hh" // for data types

/* externs */
extern int mobile_count;

#define IS_VALID(data)          ((data) != nullptr && (data)->valid)
#define VALIDATE(data)          ((data)->valid = TRUE)
#define INVALIDATE(data)        ((data)->valid = FALSE)

// for character stats
#define DEFENSE_MOD_MEM_SIZE (sizeof(sh_int) * 32)
#define APPLY_CACHE_MEM_SIZE (sizeof(int) * MAX_ATTR)

/* descriptor recycling */
Descriptor	*new_descriptor args( (void) );
void	free_descriptor args( (Descriptor *d) );

/* char gen data recycling */
Customize 	*new_gen_data args( (void) );
void	free_gen_data args( (Customize * gen) );

/* character recyling */
Player	*new_pcdata args( (void) );
void	free_pcdata args( (Player *pcdata) );


/* mob id and memory procedures */
long 	get_pc_id args( (void) );
long	get_mob_id args( (void) );


/* coordinate recycling
WM_COORD_DATA	*new_coord args((void));
void		free_coord args((WM_COORD_DATA *coord));
*/

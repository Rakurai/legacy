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

#include "merc.h" // for data types

/* externs */
extern int mobile_count;

#define IS_VALID(data)          ((data) != NULL && (data)->valid)
#define VALIDATE(data)          ((data)->valid = TRUE)
#define INVALIDATE(data)        ((data)->valid = FALSE)

// for character stats
#define DEFENSE_MOD_MEM_SIZE (sizeof(sh_int) * 32)
#define APPLY_CACHE_MEM_SIZE (sizeof(int) * MAX_ATTR)

/* note recycling */
NOTE_DATA	*new_note args( (void) );
void	free_note args( (NOTE_DATA *note) );

/* descriptor recycling */
DESCRIPTOR_DATA	*new_descriptor args( (void) );
void	free_descriptor args( (DESCRIPTOR_DATA *d) );

/* char gen data recycling */
GEN_DATA 	*new_gen_data args( (void) );
void	free_gen_data args( (GEN_DATA * gen) );

/* extra descr recycling */
EXTRA_DESCR_DATA	*new_extra_descr args( (void) );
void	free_extra_descr args( (EXTRA_DESCR_DATA *ed) );

/* affect recycling */
AFFECT_DATA	*new_affect args( (void) );
void	free_affect args( (AFFECT_DATA *af) );

/* object recycling */
OBJ_DATA	*new_obj args( (void) );
void	free_obj args( (OBJ_DATA *obj) );

/* character recyling */
CHAR_DATA	*new_char args( (void) );
void	free_char args( (CHAR_DATA *ch) );
PC_DATA	*new_pcdata args( (void) );
void	free_pcdata args( (PC_DATA *pcdata) );


/* mob id and memory procedures */
long 	get_pc_id args( (void) );
long	get_mob_id args( (void) );
MEM_DATA	*new_mem_data args( (void) );
void	free_mem_data args( ( MEM_DATA *memory) );
MEM_DATA	*find_memory args( (MEM_DATA *memory, long id) );

/* war recycling */
WAR_DATA	*new_war args((void));
void		free_war args((WAR_DATA *war));

/* war opponent recycling */
OPP_DATA	*new_opp args((void));
void		free_opp args((OPP_DATA *opp));

/* war event recycling */
EVENT_DATA	*new_event args((void));
void		free_event args((EVENT_DATA *event));

/* merc recycling */
MERC_DATA	*new_merc args((void));
void		free_merc args((MERC_DATA *merc));

/* merc offer recycling */
OFFER_DATA	*new_offer args((void));
void		free_offer args((OFFER_DATA *offer));

/* duel data recycling */
DUEL_DATA	*new_duel args((void));
void		free_duel args((DUEL_DATA *duel));

/* coordinate recycling
WM_COORD_DATA	*new_coord args((void));
void		free_coord args((WM_COORD_DATA *coord));
*/

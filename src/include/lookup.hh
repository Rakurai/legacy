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

Clan *clan_vnum_lookup	args((int vnum));
Clan *	clan_lookup		args((const String& name));
int		position_lookup		args((const String& name));
int 	sex_lookup		args((const String& name));
int 	size_lookup		args((const String& name));
String	condition_lookup	args((int condition));
int		liq_lookup		args((const String& name));
String	sector_lookup		args((int type));
int     liq_lookup      args( ( const String& name) );
int     weapon_lookup   args( ( const String& name) );
int     item_lookup     args( ( const String& name) );
int     attack_lookup   args(( const String& name) );
int     race_lookup     args(( const String& name) );
int     class_lookup    args(( const String& name) );
int     deity_lookup    args(( const String& name) );

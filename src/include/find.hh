#pragma once

class Character;
class Object;
class Room;
class String;


/* find.c */
Character *	get_mob_here		args((Character *ch, const String& argument, int vis));
Character *	get_mob_area		args((Character *ch, const String& argument, int vis));
Character *	get_mob_world		args((Character *ch, const String& argument, int vis));
Character *	get_char_here		args((Character *ch, const String& argument, int vis));
Character * get_char_room       args((Character *ch, Room *room, const String& argument, int vis));
Character *	get_char_area		args((Character *ch, const String& argument, int vis));
Character *	get_char_world		args((Character *ch, const String& argument, int vis));
Character *	get_player_here		args((Character *ch, const String& argument, int vis));
Character *	get_player_area		args((Character *ch, const String& argument, int vis));
Character *	get_player_world	args((Character *ch, const String& argument, int vis));
Object *	get_obj_list		args((Character *ch, const String& argument, Object *list));
Object *	get_obj_carry		args((Character *ch, const String& argument));
Object *	get_obj_wear		args((Character *ch, const String& argument));
Object *	get_obj_here		args((Character *ch, const String& argument));
Object *	get_obj_world		args((Character *ch, const String& argument));

#pragma once

class Character;
class Object;
class Room;
class String;


/* find.c */
Character *	get_mob_here(Character *ch, const String& argument, int vis);
Character *	get_mob_area(Character *ch, const String& argument, int vis);
Character *	get_mob_world(Character *ch, const String& argument, int vis);
Character *	get_char_here(Character *ch, const String& argument, int vis);
Character * get_char_room(Character *ch, Room *room, const String& argument, int vis);
Character *	get_char_area(Character *ch, const String& argument, int vis);
Character *	get_char_world(Character *ch, const String& argument, int vis);
Character *	get_player_here(Character *ch, const String& argument, int vis);
Character *	get_player_area(Character *ch, const String& argument, int vis);
Character *	get_player_world(Character *ch, const String& argument, int vis);
Object *	get_obj_list(Character *ch, const String& argument, Object *list);
Object *	get_obj_carry(Character *ch, const String& argument);
Object *	get_obj_wear(Character *ch, const String& argument);
Object *	get_obj_here(Character *ch, const String& argument);
Object *	get_obj_world(Character *ch, const String& argument);

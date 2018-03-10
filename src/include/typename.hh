#pragma once

#include "String.hh"

class Character;
class Object;
class Flags;

/* typename.c */
String weapon_name     args(( int weapon_Type) );
String item_name       args(( int item_type) );
String item_type_name  args(( Object *obj ) );
String affect_loc_name args(( int location ) );
String dam_type_name   args(( int type ) );
String extra_bit_name  args(( const Flags& extra_flags ) );
String wiz_bit_name    args(( const Flags& wiz_flags ) );
String wear_bit_name   args(( const Flags& wear_flags ) );
String act_bit_name    args(( const Flags& act_flags, bool npc ) );
String room_bit_name   args(( const Flags& room_flags ));
String plr_bit_name    args(( const Flags& plr_flags ) );
String off_bit_name    args(( const Flags& off_flags ) );
String imm_bit_name    args(( const Flags& flags ) );
String form_bit_name   args(( const Flags& form_flags ) );
String part_bit_name   args(( const Flags& part_flags ) );
String weapon_bit_name args(( const Flags& weapon_flags ) );
String comm_bit_name   args(( const Flags& comm_flags ) );
String revoke_bit_name args(( const Flags& revoke_flags ) );
String cgroup_bit_name args(( const Flags& flags ));
String censor_bit_name args(( const Flags& censor_flags ) );
String cont_bit_name   args(( const Flags& cont_flags) );
String get_color_name      args((int color, int bold));
String get_color_code      args((int color, int bold));
String get_custom_color_name   args((Character *ch, int slot));
String get_custom_color_code   args((Character *ch, int slot));

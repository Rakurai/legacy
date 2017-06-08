#pragma once

#include "declare.h"

/* typename.c */
String weapon_name     args(( int weapon_Type) );
String item_name       args(( int item_type) );
String item_type_name  args(( Object *obj ) );
String affect_loc_name args(( int location ) );
String dam_type_name   args(( int type ) );
String extra_bit_name  args(( int extra_flags ) );
String wiz_bit_name    args(( int wiz_flags ) );
String wear_bit_name   args(( int wear_flags ) );
String act_bit_name    args(( int act_flags, bool npc ) );
String room_bit_name   args(( int room_flags ));
String plr_bit_name    args(( int plr_flags ) );
String off_bit_name    args(( int off_flags ) );
String imm_bit_name    args(( int flags ) );
String form_bit_name   args(( int form_flags ) );
String part_bit_name   args(( int part_flags ) );
String weapon_bit_name args(( int weapon_flags ) );
String comm_bit_name   args(( int comm_flags ) );
String revoke_bit_name args(( int revoke_flags ) );
String cgroup_bit_name args(( int flags ));
String censor_bit_name args(( int censor_flags ) );
String cont_bit_name   args(( int cont_flags) );
String get_color_name      args((int color, int bold));
String get_color_code      args((int color, int bold));
String get_custom_color_name   args((Character *ch, int slot));
String get_custom_color_code   args((Character *ch, int slot));

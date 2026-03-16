#pragma once

#include "String.hh"

class Character;
class Object;
class Flags;

/* typename.c */
const String weapon_name(int weapon_type);
const String item_name(int item_type);
const String item_type_name(Object *obj);
const String affect_loc_name(int location);
const String dam_type_name(int type);
const String extra_bit_name(const Flags& flags);
const String wiz_bit_name(const Flags& flags);
const String wear_bit_name(const Flags& flags);
const String act_bit_name(const Flags& flags, bool npc);
const String room_bit_name(const Flags& flags);
const String plr_bit_name(const Flags& flags);
const String off_bit_name(const Flags& flags);
const String imm_bit_name(const Flags& flags);
const String form_bit_name(const Flags& flags);
const String part_bit_name(const Flags& flags);
const String weapon_bit_name(const Flags& flags);
const String comm_bit_name(const Flags& flags);
const String revoke_bit_name(const Flags& flags);
const String cgroup_bit_name(const Flags& flags);
const String censor_bit_name(const Flags& flags);
const String cont_bit_name(const Flags& flags);
const String get_color_name(int color, int bold);
const String get_color_code(int color, int bold);
const String get_custom_color_name(Character *ch, int slot);
const String get_custom_color_code(Character *ch, int slot);

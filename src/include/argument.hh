#pragma once

#include "Flags.hh"

class String;

int number_argument(const String& argument, String& arg);
Flags::Bit entity_argument(const String& argument, String& arg);
int mult_argument(const String& argument, String& arg);
const char *one_argument(const String& argument, String& arg);


/**
 * @defgroup EntityArgumentValues Values returned by entity_argument()
 * @{
 */
constexpr Flags::Bit ENTITY_P = Flags::A;
constexpr Flags::Bit ENTITY_M = Flags::B;
//constexpr Flags::Bit ENTITY_C = ENTITY_P | ENTITY_M;
constexpr Flags::Bit ENTITY_O = Flags::C;
constexpr Flags::Bit ENTITY_R = Flags::D;

/* More entity types: Mobs, objects and rooms as Vnums */
constexpr Flags::Bit ENTITY_VM = Flags::E;   /**< Mob Vnum */
constexpr Flags::Bit ENTITY_VO = Flags::F;   /**< Obj Vnum */
constexpr Flags::Bit ENTITY_VR = Flags::G;   /**< Room Vnum */
/** @} */

#pragma once

#include "Flags.hh"

class String;

int number_argument(const char * argument, char* arg);
Flags::Bit entity_argument(const char * argument, char* arg);
int mult_argument(const char * argument, char* arg);
const char *one_argument(const char * argument, char* arg);

int number_argument(const String& argument, String& arg);
Flags::Bit entity_argument(const String& argument, String& arg);
int mult_argument(const String& argument, String& arg);
const char *one_argument(const String& argument, String& arg);


/*
 * Values returned by entity_argument()
 * These represent players, mobiles, characters (either),
 * objects and rooms, respectively.
 */
#define ENTITY_P (Flags::A)
#define ENTITY_M (Flags::B)
#define ENTITY_C (ENTITY_P|ENTITY_M)
#define ENTITY_O (Flags::C)
#define ENTITY_R (Flags::D)

/* More entity types: Mobs, objects and rooms as Vnums */
#define ENTITY_VM (Flags::E)   /* Mob Vnum */
#define ENTITY_VO (Flags::F)   /* Obj Vnum */
#define ENTITY_VR (Flags::G)   /* Room Vnum */

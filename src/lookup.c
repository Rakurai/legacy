/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
        &&  !str_prefix1(name,flag_table[flag].name))
            return flag;
    }

    return -1;
}

CLAN_DATA *clan_lookup(const char *name)
{	CLAN_DATA *iterator;

	if (name[0]=='\0')
		return NULL;

	iterator=clan_table_head->next;
	while (iterator!=clan_table_tail)
	{	//printf("%s\n",iterator->name);
		if ( !str_prefix1( name, iterator->name ) )
            	{	return iterator;
            	}
            	iterator=iterator->next;
    	}
    	return NULL;
}

int position_lookup (const char *name)
{
   int pos;

   for (pos = 0; position_table[pos].name != NULL; pos++)
   {
        if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
        &&  !str_prefix1(name,position_table[pos].name))
            return pos;
   }

   return -1;
}

int sex_lookup (const char *name)
{
   int sex;

   for (sex = 0; sex_table[sex].name != NULL; sex++)
   {
        if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
        &&  !str_prefix1(name,sex_table[sex].name))
            return sex;
   }

   return -1;
}

int size_lookup (const char *name)
{
   int size;

   for ( size = 0; size_table[size].name != NULL; size++)
   {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix1( name,size_table[size].name))
            return size;
   }

   return -1;
}
char *condition_lookup(int condition)
{
    if (condition >= 100)
        return "perfect";
    else if (condition >= 90)
        return "good";
    else if (condition >= 75)
        return "average";
    else if (condition >= 50)
        return "worn";
    else if (condition >= 25)
        return "damaged";
    else if (condition >= 10)
        return "broken";
    else if (condition >= 0)
        return "ruined";
    else if (condition >= -1)
        return "indestructable";
    else
        return "unknown";
}

char *sector_lookup(int type) {
  int i;
  for (i = 0; sector_table[i].name != NULL; i++)
    if (sector_table[i].type == type)
      return sector_table[i].name;
  return "unknown";
}

int drink_lookup (const char *name)
{
	int pos;

	for (pos = 0; position_table[pos].name != NULL; pos++)
		if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
		 && !str_prefix1(name,position_table[pos].name))
			return pos;

	return -1;
}

/*
newline.c

This file contains code to strip new line
and charage returns from a string.

Licensesd under the GNU Genral Public License.
*/

#include <stdio.h>
#include <string.h>

#include "newline.h"

void Strip_Newline(char *line)
{
   int index;
   int max_length;

   max_length = strlen(line);
   for (index = 0; index < max_length; index++)
   {
     if ( (line[index] == '\r') || (line[index] == '\n') )
        line[index] = '\0';
   }
}


#pragma once

#include <vector>
#include "declare.hh"
#include "String.hh"

#define MAX_LINES	100 /* this boils down to about 1k per song */
#define MAX_GLOBAL	10  /* max songs the global jukebox can hold */

struct song_data
{
    String group;
    String name;
    String lyrics[MAX_LINES];
    int lines;
};

extern std::vector<struct song_data> song_table;

void song_update args( (void) );
void load_songs	args( (void) );

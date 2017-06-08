#pragma once

#include "declare.h"

/* channel.c */
void global_act(Character *ch, const String& message, int despite_invis, int color, long nocomm_bits);
void talk_auction(const String& argument);
void channel(Character *ch, const String& argument, int channel);
void channel_who(Character *ch, const String& channelname, int channel, int custom);
void wiznet(const String& string, Character *ch, Object *obj, long flag, long flag_skip, int min_rank);

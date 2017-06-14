#pragma once

#include "declare.h"
#include "Flags.hpp"

/* channel.c */
void global_act(Character *ch, const String& message, int despite_invis, int color, const Flags& nocomm_bits);
void talk_auction(const String& argument);
void channel(Character *ch, const String& argument, int channel);
void channel_who(Character *ch, const String& channelname, const Flags::Bit& channel, int custom);
void wiznet(const String& string, Character *ch, Object *obj, const Flags& flag, const Flags& flag_skip, int min_rank);

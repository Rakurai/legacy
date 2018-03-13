#pragma once

namespace quest {

const Quest *lookup(const String& id);

State *get_state(Player *player, const Quest *quest);
State *get_state(Player *player, int index);

void assign(Player *, const Quest *);
void progress(Player *, const Quest *);
//void complete(Player *, const Quest *);
void remove(Player *, const Quest *);
void remove(Player *, int index);

bool can_start(Player *, const Quest *);

} // namespace quest

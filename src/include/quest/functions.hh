#pragma once

namespace quest {

const Quest *lookup(const String& id);
const String list(Player *);

State *get_state(Player *player, const Quest *quest);
State *get_state(Player *player, int index);

void assign(Player *, const Quest *);
void progress(Player *, const Quest *);
//void complete(Player *, const Quest *);
void remove(Player *, const Quest *);
void remove(Player *, int index);

} // namespace quest

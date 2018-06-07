#pragma once

namespace quest {

const Quest *lookup(const String& id);

State *get_state(Player *player, const Quest *quest);
State *get_state(Player *player, int index);
State *get_state(Player *player, const String& id);

void assign(Player *, const Quest *);
void progress(Player *, const Quest *);
//void complete(Player *, const Quest *);
void remove(Player *, const Quest *);
void remove(Player *, int index);

bool can_start(Player *, const Quest *);

const Quest* is_target(const Player *, const Object *);
const Quest* is_target(const Player *, const Character *);
const String get_state_mapping(const State *, int step, const String& key);
const String get_state_mapping(Player *, const Quest *, int step, const String& key);
void set_state_mapping(State *, int step, const String& key, const String& value);
void set_state_mapping(Player *, const Quest *, int step, const String& key, const String& value);

bool test_progress_slay(Player *, Character *mob);
bool test_progress_get(Player *, Object *obj);

} // namespace quest

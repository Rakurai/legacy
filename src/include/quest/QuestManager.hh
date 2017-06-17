#pragma once

#include "declare.hh"
#include "Quest.hh"
#include "SkillQuest.hh"
#include "PointQuest.hh"

namespace quest {

class QuestManager {
public:
	QuestManager(Player& p) : player(p) {}
	virtual ~QuestManager() {
		delete quest;
		delete squest;
	}

	void update(); // update all quests
	void stop_quest(); // stop regular quest
	void stop_squest(); // stop skill quest

	bool is_target(const QuestTargetable *) const; // see if the thing is a target of any quests
	void check_complete(const QuestTargetable *); // check completion of any quests with this thing

	PointQuest *quest = nullptr;
	SkillQuest *squest = nullptr;
	sh_int nextquest = 0;
	sh_int nextsquest = 0;

	static int doubleqp; // globally accessed

private:
	QuestManager(const QuestManager&);
	QuestManager& operator=(const QuestManager&);

	Player& player;
};

} // namespace quest

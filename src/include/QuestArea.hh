#pragma once

#include "declare.hh"

class QuestArea {
public:
	QuestArea() {}
	virtual ~QuestArea() {}

	void init();

	bool open = false;
	bool pk = true; // unlimited pk
	int min_level = 0;
	int max_level = MAX_LEVEL;

	RoomPrototype *startroom = nullptr;
	Area *area = nullptr;

private:
	QuestArea(const QuestArea&);
	QuestArea& operator=(const QuestArea&);
};

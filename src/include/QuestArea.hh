#pragma once

#include "constants.hh"

class Area;
class Room;

class QuestArea {
public:
	QuestArea() {}
	virtual ~QuestArea() {}

	void init();
	const Area& area() const;

	bool open = false;
	bool pk = true; // unlimited pk
	int min_level = 0;
	int max_level = MAX_LEVEL;

	Room *startroom = nullptr;
//	const Area *area = nullptr;

private:
	QuestArea(const QuestArea&);
	QuestArea& operator=(const QuestArea&);
};

#pragma once

int count_active(Character *ch) {
	if (!ch->pcdata)
		return 0;

	int count = 0;

	for (Quest *q: ch->pcdata->quests)
		if (q.objectives.size() > 1)
			count++;

	return count;
}

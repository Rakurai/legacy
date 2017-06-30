#include <map>
#include <queue>

#include "Character.hh"
#include "Exit.hh"
#include "Logging.hh"
#include "RoomPrototype.hh"


void get_paths(
	Character *ch,
	RoomPrototype *start,
	std::set<RoomPrototype *> &goals,
	bool all_goals,
	std::map<RoomPrototype *, RoomPrototype *> &paths) {

	const std::clock_t begin_time = std::clock();

	std::queue<RoomPrototype *> frontier;
	frontier.push(start);

	paths[start] = 0;
	int steps = 0;

	while (!frontier.empty()) {
		RoomPrototype *current = frontier.front();
		frontier.pop();

		// remove the current goal and stop if we've removed them all
		// goals.erase will return the number of items removed from the set (1 or 0).
		// has same time complexity as goals.find (with sets)
		// this lets us also use an empty set for goals and search all paths
		if (goals.erase(current) // found a goal
		 && (goals.empty() // found all goals
		  || !all_goals)) // or only wanted one of the goals
			break;

		for (int i = 0; i < 6; ++i) {
			if (!current->exit[i])
				continue;

			RoomPrototype *next = current->exit[i]->u1.to_room;

			if (next == nullptr) {
				Logging::bugf("found non-null exit pointing to null room, vnum %d", current->vnum);
			}

			if (paths.find(next) == paths.end()) {
				frontier.push(next);
				paths[next] = current;
			}
		}

		++steps;
	}

	ptc(ch, "\nTook %d steps, %f seconds.\n",
		steps, float(std::clock() - begin_time) / CLOCKS_PER_SEC);
}

void spatial_search(Character *ch, RoomPrototype *start, RoomPrototype *goal) {
	const std::clock_t begin_time = std::clock();

	std::queue<RoomPrototype *> frontier;
	frontier.push(start);

	std::map<RoomPrototype *, RoomPrototype *> came_from;

	struct distance_st {
		int x = 0;
		int y = 0;
		int z = 0;
	};

	std::map<RoomPrototype *, distance_st> distance;

	came_from[start] = 0;
	distance.emplace(start, distance_st());

	int steps = 0;

	while (!frontier.empty()) {
		RoomPrototype *current = frontier.front();
		frontier.pop();

		distance_st& dist = distance[current];
		ptc(ch, "%4d visiting %d [%2d,%2d,%2d]\n",
			steps, current->vnum, dist.x, dist.y, dist.z);

		if (goal && current == goal)
			break;

		for (int i = 0; i < 6; ++i) {
			if (!current->exit[i])
				continue;

			RoomPrototype *next = current->exit[i]->u1.to_room;

			if (next == nullptr) {
				Logging::bugf("found non-null exit pointing to null room, vnum %d", current->vnum);
			}

			if (came_from.find(next) == came_from.end()) {
				frontier.push(next);
				came_from[next] = current;
				distance.emplace(next, distance[current]);
				switch (i) {
					case 0: distance[next].y++; break;
					case 1: distance[next].x++; break;
					case 2: distance[next].y--; break;
					case 3: distance[next].x--; break;
					case 4: distance[next].z++; break;
					case 5: distance[next].z--; break;
				}
			}
		}

		++steps;
	}

	float diff_time = float(std::clock() - begin_time) / CLOCKS_PER_SEC;

	ptc(ch, "\nTook %f seconds.\n", diff_time);
}

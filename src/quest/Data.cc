#include "quest/Data.hh"
#include "file.hh"
#include "Logging.hh"

namespace quest {

Data::
Data(const String& filename, FILE *fp) :
	filename(filename)
{
	while (true) {
		String key = fread_word(fp);

		if (key[0] == '*') {
			fread_to_eol(fp);
			continue;
		}

		     if (key == "name")     name = fread_string(fp);
		else if (key == "id")       id = fread_string(fp);
		else if (key == "step")     steps.push_back(Step(fp));
		else if (key == "reward")   rewards.push_back(Reward(fp));
		else if (key == "end")      break;
		else {
			Logging::bugf("Quests::Data: invalid keyword '%s' in header.", key);
			exit(1);
		}
	}
}

Data::Step::
Step(FILE *fp) :
	description(fread_string(fp))
{}

Data::Reward::
Reward(FILE *fp) :
	type(fread_word(fp)),
	what(fread_number(fp)),
	amount_min(fread_number(fp)),
	amount_max(fread_number(fp)),
	stop(!(fread_number(fp) == 0)),
	pct_chance(fread_number(fp))
{
	if (type != "gold"
	 && type != "silver"
	 && type != "xp"
	 && type != "qp"
	 && type != "sp"
	 && type != "obj"
	 && type != "prac"
	 && type != "train") {
		Logging::bugf("Quests::Data::Reward: unknown reward type '%s'.", type);
		exit(1);
	}

	if (amount_min < 1 || amount_max < amount_min) {
		Logging::bugf("Quests::Data::Reward: invalid min/max amounts '%d' and '%d'.", amount_min, amount_max);
		exit(1);
	}

	if (pct_chance < 0 || pct_chance > 100) {
		Logging::bugf("Quests::Data::Reward: invalid chance '%d'.", pct_chance);
		exit(1);
	}
}

} // namespace quest

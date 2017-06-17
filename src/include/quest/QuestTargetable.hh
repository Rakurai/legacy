#pragma once

namespace quest {

class QuestTargetable {
public:
	virtual const std::string identifier() const = 0;
};

} // namespace quest

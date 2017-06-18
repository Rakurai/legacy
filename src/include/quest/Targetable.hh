#pragma once

#include <string>

namespace quest {

class Targetable {
public:
	virtual const std::string identifier() const = 0;
};

} // namespace quest

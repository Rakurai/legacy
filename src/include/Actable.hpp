#pragma once

#include <string>

class Actable {
	virtual std::string identifier() const = 0;
};

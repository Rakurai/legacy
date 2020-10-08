#pragma once

#include <string>

class Actable {
	virtual const std::string identifier() const = 0;
};

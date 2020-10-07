#pragma once

#include <string>

class Actable {
public:
	virtual ~Actable();
	virtual const std::string identifier() const = 0;
};

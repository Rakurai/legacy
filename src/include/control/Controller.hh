#pragma once

#include "declare.hh"

namespace control {

class Controller {
public:
	virtual ~Controller() {}
	
	void attach(Character *new_ch) { character = new_ch; }
	void detach() { character = nullptr; }
	virtual void update() = 0;

	Character *character = nullptr;

protected:
	Controller();

private:
	Controller(const Controller&);
	Controller& operator=(const Controller&);	
};

} // namespace control

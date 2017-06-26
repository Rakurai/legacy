#pragma once

namespace control {

#include "Controller.hh"

class MobileController : public Controller {
public:
	virtual ~MobileController() {}
	
	void update() {}

	static MobileController& getController() {
		static MobileController c;
		return c;
	}

private:
	MobileController();
	MobileController(const MobileController&);
	MobileController& operator=(const MobileController&);
};

} // namespace control

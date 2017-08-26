#pragma once

class Valid {
public:
	void validate() { _valid = true; }
	void invalidate() { _valid = false; }
	bool valid() const { return _valid; }

private:
	bool _valid = false;
};

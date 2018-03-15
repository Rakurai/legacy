#pragma once

class Garbage {
public:
	void not_garbage() { _garbage = false; }
	void make_garbage() { _garbage = true; }
	bool is_garbage() const { return _garbage; }

private:
	bool _garbage = true;
};

#pragma once

#include "ObjectValue.hpp"

class ObjectValues {
public:
	ObjectValues(const ObjectValues& v) :
        value{v[0], v[1], v[2], v[3], v[4]} {}
    ObjectValues(FILE *fp);

    ObjectValue& operator[](int index) {
    	return value[index];
    }

    ObjectValue operator[](int index) const {
    	return value[index];
    }

	ObjectValue value[5];

private:
    ObjectValues();
    ObjectValues& operator=(const ObjectValues&);
};

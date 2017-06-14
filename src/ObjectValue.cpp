#include "ObjectValue.hpp"
#include "Flags.hpp"

ObjectValue::
ObjectValue(const Flags& v) : _value(v.to_ulong()), _as_flags(true) {}

ObjectValue::
operator Flags() const {
	return Flags(_value);
}

const Flags ObjectValue::
flags() const {
	return Flags(_value);
}

ObjectValue& ObjectValue::
operator=(const Flags& rhs) {
	this->_value = rhs.to_ulong();
	return *this;
}

ObjectValue& ObjectValue::
operator+=(const ObjectValue &rhs) {
	if (_as_flags)
		_value = (Flags(_value) += Flags(rhs._value)).to_ulong();
	else
		_value += rhs._value;

	return *this;
}


ObjectValue& ObjectValue::
operator-=(const ObjectValue &rhs) {
	if (_as_flags)
		_value = (Flags(_value) -= Flags(rhs._value)).to_ulong();
	else
		_value -= rhs._value;

	return *this;
}

ObjectValue& ObjectValue::
operator^=(const Flags& rhs) {
	if (_as_flags)
		_value = (Flags(_value) ^= rhs).to_ulong();

	return *this;
}

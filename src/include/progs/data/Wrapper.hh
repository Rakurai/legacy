#pragma once

#include "String.hh"

namespace progs {
namespace data {

struct Wrapper {
	virtual ~Wrapper() {}

	// virtual copy pattern, ensure a deep copy of derived classes
	virtual Wrapper *clone() const = 0;
};

template <typename T> struct WrapperTemplate : public Wrapper {
	WrapperTemplate(T& d) : data(d) {}
	virtual ~WrapperTemplate() {}

	virtual Wrapper *clone() const {
		return new WrapperTemplate(*this);
	}

	T data;
};

template <typename T>
Wrapper *construct_wrapper(T& data) {
	return new WrapperTemplate<T>(data);
}

template <typename T>
void access_wrapper(Wrapper *wrapper, T* datap) {
	WrapperTemplate<T> *wrapperT = dynamic_cast<WrapperTemplate<T> *>(wrapper);

	if (wrapperT == nullptr)
		throw String("progs::contexts::Wrapper: wrapper not of expected type");

	*datap = wrapperT->data;
}
	
} // namespace data
} // namespace progs

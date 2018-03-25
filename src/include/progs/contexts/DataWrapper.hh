#pragma once

#include "String.hh"

namespace progs {
namespace contexts {

struct DataWrapper {
	virtual ~DataWrapper() {}

	// virtual copy pattern, ensure a deep copy of derived classes
	virtual DataWrapper *clone() const = 0;
};

template <typename T> struct DataWrapperTemplate : public DataWrapper {
	DataWrapperTemplate(T d) : data(d) {}
	virtual ~DataWrapperTemplate() {}

	virtual DataWrapper *clone() const {
		return new DataWrapperTemplate(*this);
	}

	T data;
};

template <typename T>
DataWrapper *datawrapper_construct(T& data) {
	return new DataWrapperTemplate<T>(data);
}

template <typename T>
void datawrapper_access(DataWrapper *wrapper, T** datap) {
	DataWrapperTemplate<T> *wrapperT = dynamic_cast<DataWrapperTemplate<T> *>(wrapper);

	if (wrapperT == nullptr)
		throw String("progs::contexts::DataWrapper: wrapper not of expected type");

	*datap = &wrapperT->data;
}
	
} // namespace contexts
} // namespace progs

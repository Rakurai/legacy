#pragma once

#include <string>
#include <vector>
#include <cassert>

/* Implements a self-contained list of free objects of type T, to prevent
 * creating/destroying frequently used objects.  Subclass Pooled with
 * SomeClass : public Pooled<SomeClass>, where Pooled is the first
 * inherited.  DO NOT subclass a class inheriting from Pooled!  Each class
 * must have its own object pool, or new() could return the wrong size object.
 * I have placed an assert call to slap the programmer who does this.
 *
 * based on work by Thomas George, 2003, found at:
 * https://www.codeproject.com/Articles/3968/Object-Pooling-for-Generic-C-classes
 */
template <class T > class Pooled
{
public:
	Pooled() {
//		std::cout << "Pooled: default constructor" << std::endl;
	}
	virtual ~Pooled() {
//		std::cout << "Pooled: destructor" << std::endl;
	}

	static unsigned int pool_free() {
		return free_list().size();
	}

	static unsigned int& pool_allocated() {
		static unsigned int allocated;
		return allocated;
	}

	inline void *operator new( size_t stAllocateBlock) {
//		std::cout << "Pooled: new" << std::endl;

		assert(stAllocateBlock == sizeof(T));

		if (T::free_list().size() <= 0)
			T::alloc_new();

		T* p = T::free_list().back();
		T::free_list().pop_back();
		return p;
	}

	inline void operator delete( void *p ) {
//		std::cout << "Pooled: delete" << std::endl;
		T::free_list().push_back((T*)p);
	}

	// call destroy() if the entire pool is to be freed, probably only
	// useful for a temporary list
	static void destroy() {
//		std::cout << "Pooled: destroy" << std::endl;
		auto first = T::free_list().begin();
		auto last = T::free_list().end();

		while (first != last)
		{
			T* p = *first;
			++first;
			::delete p;
		}

		T::free_list().erase();
	}

private:
	static std::vector<T* >& free_list() {
		static std::vector<T* > m_free;
		return m_free;
	}

	static void alloc_new() {
		pool_allocated()++;
//		std::cout << "Pooled: init" << std::endl;
		T::free_list().push_back(::new T);
	}

//	static unsigned int allocated = 0;
};

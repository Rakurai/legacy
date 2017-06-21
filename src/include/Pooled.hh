#pragma once

#include <string>
#include <vector>

template <class T > class Pooled
{
public:

	Pooled() {}
	virtual ~Pooled() {}

	static std::vector<T* >& get_list()
	{
		static std::vector<T* > m_free;
		return m_free;
	}
/*
	static void __op_initialize()
	{
		init();
	}

	static void __op_finalize()
	{
		destroy();
	}
*/
	static void init()
	{
		T::get_list().push_back(::new T);
	}

	inline void *operator new( size_t stAllocateBlock)
	{
		if (T::get_list().size() <= 0)
			T::init();

		T* p = T::get_list().back();
		T::get_list().pop_back();
		return p;
	}

	inline void operator delete( void *p )
	{
		T::get_list().push_back((T*)p);
	}

	static void destroy()
	{
		auto first = T::get_list().begin();
		auto last = T::get_list().end();
		while (first != last)
		{
			T* p = *first;
			++first;
			::delete p;
		}

		T::get_list().erase();
	}
};

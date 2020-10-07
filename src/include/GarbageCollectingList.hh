#pragma once

#include <list>
#include "Garbage.hh"

template <typename T> class GarbageCollectingList
{
public:
	class iterator {
	public:
		iterator(typename std::list<T>::iterator iter, GarbageCollectingList<T>& ls) : it(iter), list(ls) {}

		iterator& operator=(const iterator& rhs) { it = rhs; return *this; }
		iterator& operator++() {
	    	do {
	    		++it;
	    	}
	    	while (it != list.end().it && (*it)->is_garbage());

	    	return *this;
		}
		friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.it == rhs.it; }
		friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

		T& operator*() { return *it; }

    private:
		typename std::list<T>::iterator it;
		GarbageCollectingList& list;
	};

    iterator begin() {
    	iterator it = iterator(container.begin(), *this);

    	if (it == end() || !(*it)->is_garbage())
    		return it;

    	return ++it;
    }

    iterator end() { return iterator(container.end(), *this); }

	GarbageCollectingList() {
		(void)static_cast<Garbage *>(static_cast<T>(0));
	}
	virtual ~GarbageCollectingList() {}

//    to be implemented:
//    typedef typename std::list<T>::const_iterator const_iterator ;
//    const_iterator cbegin() const { return container.cbegin(); }
//    const_iterator cend() const { return container.cend(); }
//    const_iterator cnext() const { ??? }

    size_t size() const { return container.size(); }

    void add(T ptr) { container.push_front(ptr); ptr->not_garbage(); }
    void remove(T ptr) { ptr->make_garbage(); }

    // called occasionally
    void delete_garbage() {
   		for (auto it = container.begin(); it != container.end(); ) {
			auto ptr = *it;
			if (!ptr->is_garbage())
				++it;
			else {
				delete ptr;
				it = container.erase(it);
			}
		}
    }

private:
	GarbageCollectingList(const GarbageCollectingList&);
	GarbageCollectingList& operator=(const GarbageCollectingList&);

	std::list<T> container;
};

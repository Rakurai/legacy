#pragma once

namespace worldmap {

template<class T>
class Quadtree {
public:
	Quadtree(unsigned int capacity) : capacity(capacity) { }

	virtual ~Quadtree() {
		for (int i = 0; i < 4; i++)
			if (children[i] != nullptr)
				delete children[i];

		// data is not owned here
	}

	void put(T *data, unsigned int x, unsigned int y) {
		if (capacity == 1) {
			this->data = data;
			return;
		}

		unsigned int index = get_child_index(x, y);

		if (children[index] == nullptr)
			children[index] = new Quadtree<T>(capacity/2);

		children[index]->put(data,
			x % (capacity/2),
			y % (capacity/2)
		);
	}

	T *get(unsigned int x, unsigned int y) const {
		if (capacity == 1)
			return data;

		unsigned int index = get_child_index(x, y);

		if (children[index] == nullptr)
			return nullptr;

		return children[index]->get(
			x % (capacity/2),
			y % (capacity/2)
		);
	}

	void remove(unsigned int x, unsigned int y) {
		if (capacity == 1) {
			data = nullptr;
			return;
		}

		unsigned int index = get_child_index(x, y);

		if (children[index] == nullptr)
			return;

		children[index]->remove(
			x % (capacity/2),
			y % (capacity/2)
		);

		if (!children[index]->is_used()) {
			delete children[index];
			children[index] = nullptr;
		}
	}

private:
	Quadtree();
	Quadtree(const Quadtree&);
	Quadtree& operator=(const Quadtree&);

	unsigned int get_child_index(unsigned int x, unsigned int y) const {
		unsigned int xchild = 2 * x / capacity; //  west = 0,  east = 1
		unsigned int ychild = 2 * y / capacity; // north = 0, south = 1
		return xchild + ychild * 2;
	}

	bool is_used() const {
		return (   data != nullptr
		 || children[0] != nullptr
		 || children[1] != nullptr
		 || children[2] != nullptr
		 || children[3] != nullptr
		);
	}

	T *data = nullptr;
	Quadtree *children[4] = {nullptr}; // NW NE SE SW
	unsigned int level = 0;
	unsigned int capacity = 0;
};

} // namespace worldmap

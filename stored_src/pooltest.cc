#include <iostream>
#include "Pooled.hh"

class Base : public Pooled<Base> {
public:
	Base() { std::cout << "Base: default constructor" << std::endl; }
	virtual ~Base() { std::cout << "Base: destructor " << std::endl; }
};

class Derived : public Base {
public:
	Derived() { std::cout << "Derived: default constructor" << std::endl; }
	virtual ~Derived() { std::cout << "Derived: destructor " << std::endl; }
	int i;
};

void print_sizes() {
	std::cout << "Base    free size = " << Base::free_list().size() << std::endl;
	std::cout << "Derived free size = " << Derived::free_list().size() << std::endl;
}

int main() {
	std::cout << "creating Base" << std::endl;
	Base *b = new Base();
	print_sizes();
	std::cout << std::endl;

	std::cout << "deleting Base" << std::endl;
	delete b;
	print_sizes();
	std::cout << std::endl;

	std::cout << "creating Derived" << std::endl;
	Derived *d = new Derived();
	print_sizes();
	std::cout << std::endl;

	std::cout << "deleting Derived" << std::endl;
	delete d;
	print_sizes();
}

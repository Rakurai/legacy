#pragma once

#include <map>
#include "progs/contexts/DataWrapper.hh"
#include "String.hh"
#include "Vnum.hh"

class Character;
class Object;

namespace progs {
namespace contexts {

class Context {
public:
	Context() {}
	virtual ~Context();
	Context(const Context& rhs);

	// must be overridden
	virtual const String type() const = 0;
	virtual const Vnum& vnum() const = 0;
	virtual bool can_see(Character *ch) const = 0;
	virtual bool can_see(Object *obj) const = 0;
	virtual void process_command(const String& command) = 0;
	virtual bool self_is_garbage() const = 0;

	const String expand_vars(const String& orig);

	template <typename T>
	void add_var(const String& key, T data) {
		vars.emplace(key, datawrapper_construct(data));
	}

	DataWrapper *get_var(const String& str) {
		auto pair = vars.find(str);

		if (pair == vars.end())
			return nullptr;

		return pair->second;
	}

	std::map<String, DataWrapper *> vars;

private:
	Context& operator=(const Context&);
};

} // namespace contexts
} // namespace progs

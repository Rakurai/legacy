#pragma once

#include <map>
#include "progs/Type.hh"
#include "progs/contexts/alias.hh"
#include "progs/data/Wrapper.hh"
#include "progs/data/Type.hh"
#include "progs/data/Bindings.hh"
#include "String.hh"
#include "Format.hh"
#include "Vnum.hh"

class Character;
class Object;

namespace progs {
namespace contexts {

class Context {
public:
	Context(const data::Bindings& b) : bindings(b) {}
	virtual ~Context();
	Context(const Context& rhs); // for delayed action in MobProgActList

	// must be overridden
	virtual const String type() const = 0;
	virtual const Vnum vnum() const = 0;
	virtual bool can_see(Character *) const = 0;
	virtual bool can_see(Object *) const = 0;
	virtual bool can_see(Room *) const = 0;
	virtual Room *in_room() const = 0;

	virtual void process_command(const String& command) = 0;
	virtual bool self_is_garbage() const = 0;

	template <typename T>
	void set_var(const String& key, data::Type type, T data) {
		if (aliases.count(key) != 0)
			throw Format::format("progs::Context::set_var: attempt to rebind alias '%s'", key);

		bindings.add(key, type);

		auto var = variables.find(key);

		if (var != variables.end()) {
			delete var->second;
			variables.erase(key);
		}

		variables.emplace(key, data::construct_wrapper(data));
	}

	template <typename T>
	void get_var(const String& key, T* datap) {
		if (aliases.count(key) != 0)
			return get_alias(*this, key, datap);

		auto pair = variables.find(key);

		if (pair == variables.end())
			throw Format::format("progs::Context::get_var: variable '%s' is undefined", key);

		return data::access_wrapper(pair->second, datap);
	}

	data::Bindings aliases;
	data::Bindings bindings;
	std::map<String, data::Wrapper *> variables;
	int current_line = 0;

private:
	Context& operator=(const Context&);
};

} // namespace contexts
} // namespace progs

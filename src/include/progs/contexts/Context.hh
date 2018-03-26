#pragma once

#include <map>
#include "progs/data/Wrapper.hh"
#include "progs/data/Type.hh"
#include "String.hh"
#include "Format.hh"
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
	virtual const Vnum vnum() const = 0;
	virtual bool can_see(Character *ch) const = 0;
	virtual bool can_see(Object *obj) const = 0;
	virtual void process_command(const String& command) = 0;
	virtual bool self_is_garbage() const = 0;

	const String expand_vars(const String& orig);

	template <typename T>
	void add_var(const String& key, data::Type type, T data) {
		if (bindings.find(key) != bindings.cend())
			throw Format::format("progs::Context::add_var: variable '%s' is already bound", key);

		bindings.emplace(key, type);
		vars.emplace(key, data::construct_wrapper(data));
	}

	template <typename T>
	void get_var(const String& key, T** datap) {
		auto pair = vars.find(key);

		if (pair == vars.end())
			throw Format::format("progs::Context::get_var: variable '%s' is undefined", key);

		return data::access_wrapper(pair->second, datap);
	}

	std::map<String, data::Type> bindings;
	std::map<String, data::Wrapper *> vars;

private:
	Context& operator=(const Context&);
};

} // namespace contexts
} // namespace progs

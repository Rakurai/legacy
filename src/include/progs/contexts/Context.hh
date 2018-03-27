#pragma once

#include <map>
#include "progs/Type.hh"
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
	virtual bool can_see(Character *ch) const = 0;
	virtual bool can_see(Object *obj) const = 0;
	virtual void process_command(const String& command) = 0;
	virtual bool self_is_garbage() const = 0;

	const String expand_vars(const String& orig);

	template <typename T>
	void set_var(const String& key, data::Type type, T data) {
		bindings.add(key, type);

		auto var = vars.find(key);

		if (var != vars.end()) {
			delete var->second;
			vars.erase(key);
		}

		vars.emplace(key, data::construct_wrapper(data));
	}

	template <typename T>
	void get_var(const String& key, T** datap) {
		auto pair = vars.find(key);

		if (pair == vars.end())
			throw Format::format("progs::Context::get_var: variable '%s' is undefined", key);

		return data::access_wrapper(pair->second, datap);
	}

	data::Bindings bindings;
	std::map<String, data::Wrapper *> vars;

private:
	Context& operator=(const Context&);
};

} // namespace contexts
} // namespace progs

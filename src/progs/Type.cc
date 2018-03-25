#include "progs/Type.hh"
#include "progs/prog_table.hh"
#include "String.hh"
#include "Format.hh"

namespace progs {

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
Type name_to_type(const String& name) {
	for (const auto& pair : prog_table)
		if (pair.second.name == name)
			return pair.first;

	throw Format::format("progs::name_to_type: invalid prog type '%s'", name);
}

const String type_to_name(Type type) {
	const auto pair = prog_table.find(type);

	if (pair == prog_table.cend())
		throw String("progs::type_to_name: unhandled prog type");

	return pair->second.name;
}

} // namespace progs

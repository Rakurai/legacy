#include "progs/symbols/declare.hh"
#include "progs/symbols/FunctionSymbol.hh"

namespace progs {
namespace symbols {

const std::map<data::Type, String> conv_to_table = {
	{ data::Type::Room,      "cv_to_room" },
	{ data::Type::Object,    "cv_to_obj" },
	{ data::Type::Character, "cv_to_char" },
	{ data::Type::Boolean,   "cv_to_bool" },
	{ data::Type::Integer,   "cv_to_int" },
	{ data::Type::String,    "cv_to_str" },
};

int get_convert_fn_index(data::Type from, data::Type to) {
	const auto pair = conv_to_table.find(to);

	if (pair == conv_to_table.cend())
		return -1;

	// got the name, make sure it exists in the defined functions
	for (unsigned int index = 0; index < fn_table.size(); index++)
		if (fn_table[index].parent_class == from
		 && fn_table[index].return_class == to
		 && fn_table[index].arg_list.size() == 0
		 && fn_table[index].name == pair->second)
			return index;

	return -1;
}

bool can_convert(data::Type from, data::Type to) {
	return get_convert_fn_index(from, to) != -1;
}

std::unique_ptr<Symbol> convert(std::unique_ptr<Symbol>& from, data::Type to_type, const data::Bindings& bindings) {
	int index = get_convert_fn_index(from->type, to_type);

	if (index == -1)
		throw Format::format("progs::symbols::convert: no implicit conversion from '%s' to '%s'",
			data::type_to_string(from->type), data::type_to_string(to_type));

	String fn = fn_table[index].name + "()";

	return parseFunctionSymbol(fn, bindings, from);
}

} // namespace symbols
} // namespace progs

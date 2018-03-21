struct fn_type {
	String name;
	VariableType return_type;
	VariableType parent_type;
	std::vector<VariableType> arg_type_list;
};

static const std::vector<fn_type> fn_table = {
	{ "name", VariableType::String, VariableType::Character, {} },
};

FunctionSymbol::
FunctionSymbol(const String& fn, std::vector<std::unique_ptr<Symbol>>& a, std::unique_ptr<Symbol> p) :
	arg_list(std::move(a)),
	parent(std::move(p))
{
	for (fn_index = 0; fn_index < fn_table.size(); ++fn_index)
		if (fn == fn_table[fn_index].name
		 && parent.type == fn_table[fn_index].parent_type)
			break;

	if (fn_index == fn_table.size())
		throw Format::format("progs::FunctionSymbol: function name '%s' not valid for parent type", fn);
}

const String FunctionSymbol::
to_string(const Context& context) const {
	return context.compute_function(fn, arg_list);
}

void * FunctionSymbol::
get_ptr(const Context& context) const {



	return context.compute_function(fn, arg_list);
}

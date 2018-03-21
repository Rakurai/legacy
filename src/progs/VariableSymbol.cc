const String VariableSymbol::
to_string(const Context& context) const {
	return context.dereference_variable(var, member_name);
}

void * VariableSymbol::
get_ptr(const Context& context) const {
	return context.dereference_variable(var, member_name);
}

#pragma once

#include <vector>
#include "Context.hh"
#include "Format.hh"

namespace progs {

class Context;

class Symbol
{
public:
	virtual ~Symbol() {}
	virtual const String to_string(const Context&) const = 0;
//	virtual const String to_string(const Context& context) const {
//		throw String("progs::Symbol::evaluate: base class evaluate called");
//	}

	// factory method
	static std::unique_ptr<Symbol> parse(String&, const String& until);

private:
	static Symbol *parseVariableSymbol(String&);
	static Symbol *parseFunctionSymbol(String&);
	static Symbol *parseIntegerSymbol(String&);
	static Symbol *parseBooleanSymbol(String&);
	static Symbol *parseStringSymbol(String&, const String& until);

	virtual void *get_ptr(const Context&) const = 0;
};

class VariableSymbol : public Symbol {
public:
	VariableSymbol(const String& v, const String& m)
		: var(v), member_name(m) {}
	virtual ~VariableSymbol() {}
	virtual const String to_string(const Context& context) const;

private:
	String var;
	String member_name;

	virtual void *get_ptr(const Context&) const;
};

class FunctionSymbol : public Symbol {
public:
	FunctionSymbol(const String&, std::vector<std::unique_ptr<Symbol>>&);
	virtual ~FunctionSymbol() {}
	virtual const String to_string(const Context& context) const;

private:
	int fn_index;
	std::vector<std::unique_ptr<Symbol>> arg_list;

	virtual void *get_ptr(const Context&) const;
};

class IntegerSymbol : public Symbol {
public:
	IntegerSymbol(int v)
		: val(v) {}
	virtual ~IntegerSymbol() {}
	virtual const String to_string(const Context&) const {
		return Format::format("%d", val);
	}

private:
	int val;

	virtual void *get_ptr(const Context&) const { return &val; }
};

class BooleanSymbol : public Symbol {
public:
	BooleanSymbol(bool v)
		: val(v) {}
	virtual ~BooleanSymbol() {}
	virtual const String to_string(const Context&) const {
		return val ? "1" : "0";
	}

private:
	bool val;

	virtual void *get_ptr(const Context&) const { return &val; }
};

class StringSymbol : public Symbol {
public:
	StringSymbol(const String& v)
		: val(v) {}
	virtual ~StringSymbol() {}
	virtual const String to_string(const Context&) const {
		return val;
	}

private:
	String val;

	virtual void *get_ptr(const Context&) const { return &val; }
};

} // namespace progs

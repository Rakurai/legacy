#pragma once

#include <vector>
#include "Context.hh"

namespace progs {

class Context;

class Symbol
{
public:
	virtual ~Symbol() {}
	virtual const String evaluate(const Context&) const;

	// factory method
	static const Symbol parse(String&);
};

class VariableSymbol : public Symbol {
public:
	VariableSymbol(String str);
	virtual ~VariableSymbol() {}
	virtual const String evaluate(const Context& context) const;

private:
	String var;
	String member_name;
};

class FunctionSymbol : public Symbol {
public:
	FunctionSymbol(String str);
	virtual ~FunctionSymbol() {}
	virtual const String evaluate(const Context& context) const;

private:
	String fn;
	std::vector<Symbol> args;
};

class IntegerSymbol : public Symbol {
public:
	IntegerSymbol(int v) : val(v) {}
	IntegerSymbol(String str);
	virtual ~IntegerSymbol() {}
	virtual const String evaluate(const Context&) const;

private:
	int val;
};

class BooleanSymbol : public Symbol {
public:
	BooleanSymbol(bool v) : val(v) {}
	BooleanSymbol(String str);
	virtual ~BooleanSymbol() {}
	virtual const String evaluate(const Context&) const;

private:
	bool val;
};

class StringSymbol : public Symbol {
public:
	StringSymbol(String v) : val(v) {}
	virtual ~StringSymbol() {}
	virtual const String evaluate(const Context&) const { return val; };

private:
	String val;
};

} // namespace progs

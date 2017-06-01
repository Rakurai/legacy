#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StringModule
#include <boost/test/unit_test.hpp>

#include "String.hpp"

BOOST_AUTO_TEST_SUITE(StringTest)

BOOST_AUTO_TEST_CASE(constructors)
{
	String a; // default
	BOOST_TEST( std::operator==(a, "") );

	String b("foo"); // from char*
	BOOST_TEST( std::operator==(b, "foo") );

	String c(std::string("foo")); // from std::string
	BOOST_TEST( std::operator==(c, "foo") );

	String d(c); // from const String&
	BOOST_TEST( std::operator==(d, "foo") );

	c[0] = 'g'; // ensure deep copy
	BOOST_TEST( std::operator==(d, "foo") );
}

BOOST_AUTO_TEST_CASE(assignment)
{
	String a = "foo"; // initial assignment
	BOOST_TEST( std::operator==(a, "foo") );

	a = 'g'; // from char
	BOOST_TEST( std::operator==(a, "g") );

	a = "bar"; // from char*, converted to const String&
	BOOST_TEST( std::operator==(a, "bar") );

	a = std::string("foo"); // from std::string
	BOOST_TEST( std::operator==(a, "foo") );

	String b("bar");
	a = b; // from String
	BOOST_TEST( std::operator==(a, "bar") );

	b = "foo"; // ensure deep copy
	BOOST_TEST( std::operator==(a, "bar") );
}

BOOST_AUTO_TEST_CASE(equality) // String should be case insensitive tests
{
	String a("Foo");
	BOOST_TEST( std::operator==(a, "Foo") );
	BOOST_TEST( std::operator!=(a, "foo") ); // case sensitive

	BOOST_TEST( a == "Foo" );
	BOOST_TEST( a == "foo" ); // case insensitive
	BOOST_TEST( !(a != "Foo") );
	BOOST_TEST( !(a != "foo") );
}

BOOST_AUTO_TEST_CASE(erase)
{
	String a("qwerty");
	a.erase(4);
	BOOST_TEST( a == "qwer" );
	a.erase(0, 1);
	BOOST_TEST( a == "wer" );
	a.erase(1, 1);
	BOOST_TEST( a == "wr" );
	a.erase();
	BOOST_TEST( a.empty() );

	a = "foo";
	a.clear();
	BOOST_TEST( a.empty() );
}

BOOST_AUTO_TEST_CASE(search)
{
	String a("abracadabra");
	BOOST_TEST( a.find_nth(1, 'a') == 0 );
	BOOST_TEST( a.find_nth(2, 'a') == 3 );
	BOOST_TEST( a.find_nth(3, 'a') == 5 );
	BOOST_TEST( a.find_nth(4, 'a') == 7 );
	BOOST_TEST( a.find_nth(5, 'a') == 10 );
	BOOST_TEST( a.find_nth(6, 'a') == std::string::npos );
}

BOOST_AUTO_TEST_CASE(substr)
{
	String a("abracadabra");
	BOOST_TEST( a.substr(1) == "bracadabra" );
	BOOST_TEST( a.substr(2) == "racadabra" );
	BOOST_TEST( a.substr(2, 5) == "racad" );
	BOOST_TEST( a.substr(0, 5) == "abrac" );
}

BOOST_AUTO_TEST_CASE(capitalize)
{
	String a("foo"), b("  foo bar");
	BOOST_TEST( std::operator==(a.capitalize(), "Foo") );
	BOOST_TEST( std::operator==(b.capitalize(), "  Foo bar") );
}

BOOST_AUTO_TEST_CASE(strip)
{
	String a("foo"), b("  bar  "), c("  ");
	BOOST_TEST( a.lstrip() == "foo" );
	BOOST_TEST( a.rstrip() == "foo" );
	BOOST_TEST( a.strip() == "foo" );
	BOOST_TEST( b.lstrip() == "bar  " );
	BOOST_TEST( b.rstrip() == "  bar" );
	BOOST_TEST( b.strip() == "bar" );
	BOOST_TEST( c.lstrip() == "" );
	BOOST_TEST( c.rstrip() == "" );
	BOOST_TEST( c.strip() == "" );

	a = "abracadabra";
	BOOST_TEST( a.strip("b") == "abracadabra" );
	BOOST_TEST( a.strip("a") == "bracadabr" );
	BOOST_TEST( a.strip("ab") == "racadabr" );
}

BOOST_AUTO_TEST_CASE(split)
{
	// two forms:
	// String split(const char *chars) - returns word
	// String split(String& word, const char *chars) - assigns word, returns remainder
	//
	// word will always be stripped of all chars in "chars", right and left
	// remainder will be left stripped (with lstrip), but may have trailing
	String a, word;

	// empty input
	BOOST_TEST( a.lsplit() == "" );
	BOOST_TEST( a.rsplit() == "" );
	BOOST_TEST( a.lsplit(word) == "" );
	BOOST_TEST( word == "" );	
	BOOST_TEST( a.rsplit(word) == "" );
	BOOST_TEST( word == "" );	

	// input with just "chars"
	a = "  ";
	BOOST_TEST( a.lsplit() == "" );
	BOOST_TEST( a.rsplit() == "" );
	BOOST_TEST( a.lsplit(word) == "" );
	BOOST_TEST( word == "" );
	BOOST_TEST( a.rsplit(word) == "" );
	BOOST_TEST( word == "" );

	// single word input
	a = "foo";
	BOOST_TEST( a.lsplit() == "foo" );
	BOOST_TEST( a.rsplit() == "foo" );
	BOOST_TEST( a.lsplit(word) == "" );
	BOOST_TEST( word == "foo" );	
	BOOST_TEST( a.rsplit(word) == "" );
	BOOST_TEST( word == "foo" );	

	// whitespace surrounding, word should always be stripped
	a = "  foo ";
	BOOST_TEST( a.lsplit() == "foo" );
	BOOST_TEST( a.rsplit() == "foo" );
	BOOST_TEST( a.lsplit(word) == "" );
	BOOST_TEST( word == "foo" );	
	BOOST_TEST( a.rsplit(word) == "" );
	BOOST_TEST( word == "foo" );	

	// multiple word input
	a = "  foo bar bbq  ";
	BOOST_TEST( a.lsplit() == "foo" );
	BOOST_TEST( a.rsplit() == "bbq" );
	BOOST_TEST( a.lsplit(word) == "bar bbq  " );
	BOOST_TEST( word == "foo" );
	BOOST_TEST( a.rsplit(word) == "  foo bar" );
	BOOST_TEST( word == "bbq" );

	// multiple "chars" split parameter
	a = "abracadabra";
	BOOST_TEST( a.lsplit("b") == "a" );
	BOOST_TEST( a.lsplit("rc") == "ab" );
	BOOST_TEST( a.rsplit("ab") == "r" );
	BOOST_TEST( a.lsplit(word, "rd") == "acadabra" );
	BOOST_TEST( word == "ab" );
}

BOOST_AUTO_TEST_CASE(replace)
{
	// empty string
	String a;
	BOOST_TEST( a.replace("foo", "baz") == "" );

	// exact match
	a = "foo";
	BOOST_TEST( a.replace("foo", "bar") == "bar" );

	// doesn't exist
	BOOST_TEST( a.replace("bar", "baz") == "foo" );

	// empty with - remove
	BOOST_TEST( a.replace("foo", "") == "" );

	// empty what - don't match
	BOOST_TEST( a.replace("", "baz") == "foo" );

	// case sensitive?
	BOOST_TEST( a.replace("Foo", "baz") == "Foo" );

	// multiple replacement
	a = "foo foo bar foo bar";
	BOOST_TEST( a.replace("foo", "baz", 1) == "baz foo bar foo bar" );
	BOOST_TEST( a.replace("foo", "baz", 2) == "baz baz bar foo bar" );
	BOOST_TEST( a.replace("foo", "baz")    == "baz baz bar baz bar" );

	// too many times
	BOOST_TEST( a.replace("foo", "baz", 5) == "baz baz bar baz bar" );

	// words together
	a = "foofoobarfoo";
	BOOST_TEST( a.replace("foo", "baz")    == "bazbazbarbaz" );

	// single chars
	BOOST_TEST( a.replace("o", "a")    == "faafaabarfaa" );

	// different lengths
	BOOST_TEST( a.replace("o", "aa")    == "faaaafaaaabarfaaaa" );

	// bad recursion?
	a = "foo";
	BOOST_TEST( a.replace("o", "oo")    == "foooo" );
}

BOOST_AUTO_TEST_SUITE_END()

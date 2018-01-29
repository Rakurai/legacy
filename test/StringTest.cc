#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StringModule
#include <boost/test/unit_test.hpp>

#include "String.hh"

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
	String b("");
	BOOST_TEST( std::operator==(a, "Foo") );
	BOOST_TEST( std::operator!=(a, "foo") ); // case sensitive

	BOOST_TEST( a == "Foo" );
	BOOST_TEST( a == "foo" ); // case insensitive
	BOOST_TEST( !(a != "Foo") );
	BOOST_TEST( !(a != "foo") );

	BOOST_TEST( a.is_prefix_of(a) ); // match vs self
	BOOST_TEST( a.is_prefix_of(a, 0) );
	BOOST_TEST( a.is_prefix_of(a, 3) );
	BOOST_TEST( !a.is_prefix_of(a, 4) ); // too many required chars
	BOOST_TEST( a.is_prefix_of("foo") ); 
	BOOST_TEST( !a.is_prefix_of("Fod") ); 
	BOOST_TEST( a.is_prefix_of("Foobar") ); 
	BOOST_TEST( a.is_prefix_of("Foo bar") ); 
	BOOST_TEST( !a.is_prefix_of("Fod bar") ); 
	BOOST_TEST( !a.is_prefix_of("Fo") ); // not enough chars
	BOOST_TEST( !a.is_prefix_of("Fo", 0) ); // not enough chars
	BOOST_TEST( !a.is_prefix_of("Fo", 2) ); // not enough chars
	BOOST_TEST( !a.is_prefix_of("Fo", 3) ); // not enough chars
	BOOST_TEST( !a.is_prefix_of("Fo", 4) ); // not enough chars

	BOOST_TEST( !b.is_prefix_of("Foo") ); // default requires 1 char
	BOOST_TEST( b.is_prefix_of("Foo", 0) );

	BOOST_TEST( a.is_infix_of(a) ); // match vs self
	BOOST_TEST( a.is_infix_of(a, 0) );
	BOOST_TEST( a.is_infix_of(a, 3) );
	BOOST_TEST( !a.is_infix_of(a, 4) ); // too many required chars
	BOOST_TEST( a.is_infix_of("foo") ); 
	BOOST_TEST( !a.is_infix_of("Fod") ); 
	BOOST_TEST( a.is_infix_of("barFoobar") ); 
	BOOST_TEST( a.is_infix_of("bar Foo bar") ); 
	BOOST_TEST( !a.is_infix_of("bar Fod bar") ); 
	BOOST_TEST( a.is_infix_of("FooBar") ); 
	BOOST_TEST( a.is_infix_of("BarFoo") );
	BOOST_TEST( !a.is_infix_of("Fo") ); // not enough chars
	BOOST_TEST( !a.is_infix_of("Fo", 0) ); // not enough chars
	BOOST_TEST( !a.is_infix_of("Fo", 2) ); // not enough chars
	BOOST_TEST( !a.is_infix_of("Fo", 3) ); // not enough chars
	BOOST_TEST( !a.is_infix_of("Fo", 4) ); // not enough chars

	BOOST_TEST( !b.is_infix_of("Foo") ); // default requires 1 char
	BOOST_TEST( b.is_infix_of("Foo", 0) );

	BOOST_TEST( a.is_suffix_of(a) ); // match vs self
	BOOST_TEST( a.is_suffix_of(a, 0) );
	BOOST_TEST( a.is_suffix_of(a, 3) );
	BOOST_TEST( !a.is_suffix_of(a, 4) ); // too many required chars
	BOOST_TEST( a.is_suffix_of("foo") ); 
	BOOST_TEST( !a.is_suffix_of("Fod") ); 
	BOOST_TEST( a.is_suffix_of("bar Foo") ); 
	BOOST_TEST( !a.is_suffix_of("FooBar") ); 
	BOOST_TEST( a.is_suffix_of("BarFoo") );
	BOOST_TEST( !a.is_suffix_of("Fo") ); // not enough chars
	BOOST_TEST( !a.is_suffix_of("Fo", 0) ); // not enough chars
	BOOST_TEST( !a.is_suffix_of("Fo", 2) ); // not enough chars
	BOOST_TEST( !a.is_suffix_of("Fo", 3) ); // not enough chars
	BOOST_TEST( !a.is_suffix_of("Fo", 4) ); // not enough chars

	BOOST_TEST( !b.is_suffix_of("Foo") ); // default requires 1 char
	BOOST_TEST( b.is_suffix_of("Foo", 0) );

	// has_*fix just calls args of is_*fix_of backwards, so no extra tests here
}

BOOST_AUTO_TEST_CASE(has_words)
{
	// empty string
	String a;
	BOOST_TEST( !a.has_words("") );
	BOOST_TEST( !a.has_words(" ") );
	BOOST_TEST( !a.has_words("foo") );
	BOOST_TEST( !a.has_words("foo bar") );

	// basic tests
	a = "the quick brown fox jumped over the lazy dog";
	BOOST_TEST( !a.has_words("") );
	BOOST_TEST( !a.has_words(" ") );
	BOOST_TEST( !a.has_words("foo") );
	BOOST_TEST( !a.has_words("foo bar") );
	BOOST_TEST( a.has_words("the") );
	BOOST_TEST( a.has_words("the quick") );
	BOOST_TEST( a.has_words("brown quick") );
	BOOST_TEST( a.has_words("dog") );
	BOOST_TEST( !a.has_words("foo dog") );

	// prefix match by default
	BOOST_TEST( a.has_words("jump") );
	BOOST_TEST( a.has_words("jump over") );
	BOOST_TEST( !a.has_words("quicker") );
	BOOST_TEST( !a.has_words("quicker over") );
	BOOST_TEST( !a.has_words("jump", true) );
	BOOST_TEST( !a.has_words("jump over", true) );
	BOOST_TEST( !a.has_words("quicker", true) );
	BOOST_TEST( !a.has_words("quicker over", true) );

	// case-insensitive
	a = "Foo bar";
	BOOST_TEST( a.has_words("foo") );
	BOOST_TEST( a.has_words("Bar") );
	BOOST_TEST( a.has_words("FOO BAR") );
	BOOST_TEST( a.has_words("  foo ") );
	BOOST_TEST( a.has_words(" foo   bar ") );

	// weird spacing
	a = "   foo   bar  ";
	BOOST_TEST( !a.has_words("") );
	BOOST_TEST( !a.has_words(" ") );
	BOOST_TEST( a.has_words("foo") );
	BOOST_TEST( a.has_words("foo bar") );
	BOOST_TEST( a.has_words("  foo ") );
	BOOST_TEST( a.has_words(" foo   bar ") );
}

BOOST_AUTO_TEST_CASE(identities)
{
	BOOST_TEST( String("1").is_number() );
	BOOST_TEST( String("123").is_number() );
	BOOST_TEST( String("+1").is_number() );
	BOOST_TEST( String("-1").is_number() );
	BOOST_TEST( String("+123").is_number() );

	BOOST_TEST( !String("").is_number() );
	BOOST_TEST( !String("a").is_number() );
	BOOST_TEST( !String("+a").is_number() );
	BOOST_TEST( !String("-a").is_number() );
	BOOST_TEST( !String("abc").is_number() );
	BOOST_TEST( !String("1.").is_number() );
	BOOST_TEST( !String("1a").is_number() );
	BOOST_TEST( !String("1 a").is_number() );
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

	// out of bounds
	a = "qwerty";
	a.erase(4, 20);
	BOOST_TEST( a == "qwer" );
	a.erase(5);
	BOOST_TEST( a == "qwer" );
	a.erase(5, 1);
	BOOST_TEST( a == "qwer" );
	a.clear();
	a.erase(0);
	BOOST_TEST( a.empty() );
	a.erase(1);
	BOOST_TEST( a.empty() );

	a = "foo";
	a.clear();
	BOOST_TEST( a.empty() );
}

BOOST_AUTO_TEST_CASE(search)
{
	String a; // empty string
	BOOST_TEST( a.find("") == 0 );
	BOOST_TEST( a.find("", 1) == std::string::npos );
	BOOST_TEST( a.find("a") == std::string::npos );

	a = "aaa"; // empty search term
	BOOST_TEST( a.find("") == 0 );
	BOOST_TEST( a.find("", 4) == std::string::npos );

	a = "jabberwocky"; // basic single char tests
	BOOST_TEST( a.find("j") == 0 );
	BOOST_TEST( a.find("a") == 1 );
	BOOST_TEST( a.find("k") == 9 );
	BOOST_TEST( a.find("y") == 10 );
	BOOST_TEST( a.find("z") == std::string::npos );
	BOOST_TEST( a.find("b", 1) == 2 );
	BOOST_TEST( a.find("b", 2) == 2 );
	BOOST_TEST( a.find("b", 3) == 3 );

	// word searching
	BOOST_TEST( a.find("bb") == 2 );
	BOOST_TEST( a.find("bb", 1) == 2 );
	BOOST_TEST( a.find("bb", 5) == std::string::npos );

	// find nth term, single char in group
	a = "aaaaa";
	BOOST_TEST( a.find_nth(1, "a") == 0 );
	BOOST_TEST( a.find_nth(2, "a") == 1 );
	BOOST_TEST( a.find_nth(3, "a") == 2 );
	BOOST_TEST( a.find_nth(4, "a") == 3 );
	BOOST_TEST( a.find_nth(5, "a") == 4 );

	// clusters of repeated letters
	BOOST_TEST( a.find_nth(1, "aa") == 0 );
	BOOST_TEST( a.find_nth(2, "aa") == 2 );
	BOOST_TEST( a.find_nth(3, "aa") == std::string::npos );

	// correctly identifying letters and words
	a = "abracadabra";
	BOOST_TEST( a.find_nth(1, "a") == 0 );
	BOOST_TEST( a.find_nth(2, "a") == 3 );
	BOOST_TEST( a.find_nth(3, "a") == 5 );
	BOOST_TEST( a.find_nth(4, "a") == 7 );
	BOOST_TEST( a.find_nth(5, "a") == 10 );
	BOOST_TEST( a.find_nth(6, "a") == std::string::npos );
	BOOST_TEST( a.find_nth(2, "br") == 8 );
	BOOST_TEST( a.find_nth(2, "zap") == std::string::npos );
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

BOOST_AUTO_TEST_CASE(uncolor)
{
	String a;
	BOOST_TEST( a.uncolor() == "" );

	a = "abc def";
	BOOST_TEST( a.uncolor() == a );

	a = "{";
	BOOST_TEST( a.uncolor() == "" );

	a = "{abc d{ef{x";
	BOOST_TEST( a.uncolor() == "bc df" );

	a = "{{a c{{{b";
	BOOST_TEST( a.uncolor() == "{a c{" );
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

	// case insensitive?
	BOOST_TEST( a.replace("Foo", "baz") == "baz" );

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

BOOST_AUTO_TEST_CASE(insert)
{
	String a;
	BOOST_TEST( a.insert("", 0) == "" );
	BOOST_TEST( a.insert("", 1) == "" );
	BOOST_TEST( a.insert("foo", 0) == "foo" );
	BOOST_TEST( a.insert("foo", 1) == "foo" );

	a = "foo";
	BOOST_TEST( a.insert("", 0) == "foo" );
	BOOST_TEST( a.insert("", 1) == "foo" );
	BOOST_TEST( a.insert("", 3) == "foo" );
	BOOST_TEST( a.insert("", 4) == "foo" );
	BOOST_TEST( a.insert("bar", 0) == "barfoo" );
	BOOST_TEST( a.insert("bar", 1) == "fbaroo" );
	BOOST_TEST( a.insert("bar", 3) == "foobar" );
	BOOST_TEST( a.insert("bar", 4) == "foobar" );
}

BOOST_AUTO_TEST_CASE(center)
{
	String a;
	BOOST_TEST( a.center(0) == "" );
	BOOST_TEST( a.center(1) == " " );
	BOOST_TEST( a.center(2) == "  " );

	a = "foo";
	BOOST_TEST( a.center(0) == "" );
	BOOST_TEST( a.center(1) == "f" );
	BOOST_TEST( a.center(3) == "foo" );
	BOOST_TEST( a.center(4) == "foo " );
	BOOST_TEST( a.center(5) == " foo " );
	BOOST_TEST( a.center(6) == " foo  " );
	BOOST_TEST( a.center(7) == "  foo  " );

	a = "{cf{co{co{x";
	BOOST_TEST( a.center(0) == "{c" );
	BOOST_TEST( a.center(1) == "{cf{c" );
	BOOST_TEST( a.center(2) == "{cf{co{c" );
	BOOST_TEST( a.center(3) == "{cf{co{co{x" );
	BOOST_TEST( a.center(7) == "  {cf{co{co{x  " );
}

BOOST_AUTO_TEST_SUITE_END()

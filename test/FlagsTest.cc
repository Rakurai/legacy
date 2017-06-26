#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE FlagsModule
#include <boost/test/unit_test.hpp>

#include "Flags.hh"
#include "String.hh"

BOOST_AUTO_TEST_SUITE(FlagsTest)

#define ABC (7)
#define abc (static_cast<unsigned long>(Flags::a) \
	|static_cast<unsigned long>(Flags::b) \
	|static_cast<unsigned long>(Flags::c))


BOOST_AUTO_TEST_CASE(constructors)
{
	// default
	BOOST_TEST( Flags().to_ulong() == 0 );

	// from unsigned long
	BOOST_TEST( Flags(1).to_ulong() == 1 );

	// from Flags::Bit
	BOOST_TEST( Flags(ABC).to_ulong() == ABC );

	// from Flags
	Flags a(ABC);
	BOOST_TEST( Flags(a).to_ulong() == ABC );

	// from String
	BOOST_TEST( Flags("").to_ulong() == 0 ); // empty
	BOOST_TEST( Flags("A").to_ulong() == 1 ); // one char
	BOOST_TEST( Flags("ABC").to_ulong() == ABC ); // multiple chars
	BOOST_TEST( Flags("CBA").to_ulong() == ABC ); // any order
	BOOST_TEST( Flags("abc").to_ulong() == abc ); // lowercase
	BOOST_TEST( Flags("ABCabc").to_ulong() == ABC|abc ); // mixed

	BOOST_TEST( Flags("0").to_ulong() == 0 ); // zero
	BOOST_TEST( Flags("1").to_ulong() == 1 ); // digit
	BOOST_TEST( Flags("7").to_ulong() == ABC ); // multiple bits
	BOOST_TEST( Flags("16").to_ulong() == 16 ); // multi-digit numbers
	BOOST_TEST( Flags("17").to_ulong() == 17 ); // multi-bit, multi-digit numbers
	BOOST_TEST( Flags("7abc").to_ulong() == ABC|abc ); // digits + letters
	BOOST_TEST( Flags("a7bc").to_ulong() == ABC|abc ); // mixed
	BOOST_TEST( Flags("cba7").to_ulong() == ABC|abc ); // any order
	BOOST_TEST( Flags("a1b2c4").to_ulong() == ABC|abc ); // multiple mixes
	BOOST_TEST( Flags("1a2b4c16").to_ulong() == ABC|abc|16 ); // multiple mixes with multidigit

	BOOST_TEST( Flags("|").to_ulong() == 0 ); // two zeros
	BOOST_TEST( Flags("|0").to_ulong() == 0 ); // two zeros
	BOOST_TEST( Flags("0|").to_ulong() == 0 ); // two zeros
	BOOST_TEST( Flags("0|A").to_ulong() == 1 ); // 0 + expression
	BOOST_TEST( Flags("A|1").to_ulong() == 1 ); // expression + 0
	BOOST_TEST( Flags("A|BC").to_ulong() == ABC ); // multi-expression
	BOOST_TEST( Flags("A|B|C").to_ulong() == ABC ); // 0 + expression
}

BOOST_AUTO_TEST_CASE(representation)
{
	Flags fzero, fABC(ABC), fabc(abc), fABCabc(ABC|abc);

	BOOST_TEST( fzero.to_ulong() == 0 );
	BOOST_TEST( fABC.to_ulong() == 7 );

	BOOST_TEST( !strcmp(fzero.to_string().c_str(), "0") );
	BOOST_TEST( !strcmp(fABC.to_string().c_str(), "ABC") );
	BOOST_TEST( !strcmp(fabc.to_string().c_str(), "abc") );
	BOOST_TEST( !strcmp(fABCabc.to_string().c_str(), "ABCabc") );
}

BOOST_AUTO_TEST_CASE(assignment)
{
	Flags fzero, fABC(ABC), fabc(abc), t;

	t = fzero;
	BOOST_TEST( t.to_ulong() == 0 );
	t = fABC;
	BOOST_TEST( t.to_ulong() == ABC );
	t = fabc;
	BOOST_TEST( t.to_ulong() == abc );

	// +=
	t = fzero;
	t += fzero;
	BOOST_TEST( t.to_ulong() == 0 ); // 0 + 0
	t += fABC;
	BOOST_TEST( t.to_ulong() == ABC ); // 0 + ABC
	t += fzero;
	BOOST_TEST( t.to_ulong() == ABC ); // ABC + 0
	t += fABC;
	BOOST_TEST( t.to_ulong() == ABC ); // ABC + ABC
	t += fabc;
	BOOST_TEST( t.to_ulong() == ABC|abc ); // ABC + abc
	t += fabc;
	BOOST_TEST( t.to_ulong() == ABC|abc ); // ABCabc + abc

	// -=
	t = fABC;
	t += fabc;
	t -= fzero;
	BOOST_TEST( t.to_ulong() == ABC|abc ); // ABCabc - 0
	t -= fabc;
	BOOST_TEST( t.to_ulong() == ABC ); // ABCabc - abc
	t -= fabc;
	BOOST_TEST( t.to_ulong() == ABC ); // ABC - abc
	t -= fABC;
	BOOST_TEST( t.to_ulong() == 0 ); // ABC - ABC
	t -= fABC;
	BOOST_TEST( t.to_ulong() == 0 ); // 0 - ABC
	t -= fzero;
	BOOST_TEST( t.to_ulong() == 0 ); // 0 - 0

	// ^=
	t = fzero;
	t ^= fzero;
	BOOST_TEST( t.to_ulong() == 0 ); // 0 ^ 0
	t ^= fABC;
	BOOST_TEST( t.to_ulong() == ABC ); // 0 ^ ABC
	t ^= fABC;
	BOOST_TEST( t.to_ulong() == 0 ); // ABC ^ ABC
	t ^= fABC;
	t ^= fabc;
	BOOST_TEST( t.to_ulong() == ABC|abc ); // ABC ^ abc
	t ^= fabc;
	BOOST_TEST( t.to_ulong() == ABC ); // ABCabc ^ abc

	// clear
	t = fABC;
	t.clear();
	BOOST_TEST( t.to_ulong() == 0 );
	t.clear();
	BOOST_TEST( t.to_ulong() == 0 );
}

BOOST_AUTO_TEST_CASE(equality)
{
	Flags a, b, fzero, fABC(ABC), fabc(abc);
	BOOST_TEST( a == b ); // 0 == 0
	a = fABC;
	BOOST_TEST( a != b ); // ABC != 0
	b = fABC;
	BOOST_TEST( a == b ); // ABC == ABC
	b = fabc;
	BOOST_TEST( a != b ); // ABC != abc
	a = fzero;
	BOOST_TEST( a != b ); // 0 != abc
}

BOOST_AUTO_TEST_CASE(assembly)
{
	Flags fzero, fA(Flags::A), fAB(Flags::A|Flags::B);

	BOOST_TEST( (fzero|Flags::A) == fA );
	BOOST_TEST( (Flags::A|Flags::A) == fA );
	BOOST_TEST( (Flags::A|Flags::B) == fAB );
	BOOST_TEST( (fA|Flags::A) == fA );
	BOOST_TEST( (fA|Flags::B) == fAB );
}

BOOST_AUTO_TEST_CASE(binary_ops)
{
	Flags fzero, fABC(ABC), fabc(abc);

	BOOST_TEST( fzero + fzero == fzero );
	BOOST_TEST( fzero + fABC == fABC );
	BOOST_TEST( fABC + fzero == fABC );
	BOOST_TEST( fABC + fABC == fABC );

	BOOST_TEST( fzero - fzero == fzero );
	BOOST_TEST( fzero - fABC == fzero );
	BOOST_TEST( fABC - fzero == fABC );
	BOOST_TEST( fABC - fABC == fzero );
}

BOOST_AUTO_TEST_CASE(inclusion)
{
	Flags fzero, fA(Flags::A), fAB(Flags::A|Flags::B), fABC(ABC), fabc(abc);

	BOOST_TEST(  fzero.has_all_of(fzero) );
	BOOST_TEST( !fzero.has_all_of(fA) );
	BOOST_TEST( !fzero.has_all_of(fAB) );

	BOOST_TEST(  fA.has_all_of(fzero) ); // has zero
	BOOST_TEST(  fA.has_all_of(fA) ); // has all
	BOOST_TEST(  fAB.has_all_of(fAB) ); // has all
	BOOST_TEST( !fAB.has_all_of(fABC) ); // has two
	BOOST_TEST( !fAB.has_all_of(fabc) ); // has none

	BOOST_TEST( !fzero.has_any_of(fzero) ); // any means 'at least one'
	BOOST_TEST( !fzero.has_any_of(fA) );
	BOOST_TEST( !fzero.has_any_of(fAB) );

	BOOST_TEST( !fA.has_any_of(fzero) ); // has zero
	BOOST_TEST(  fA.has_any_of(fA) ); // has all
	BOOST_TEST(  fAB.has_any_of(fAB) ); // has all
	BOOST_TEST(  fAB.has_any_of(fABC) ); // has one
	BOOST_TEST( !fAB.has_any_of(fabc) ); // has none

	BOOST_TEST(  fzero.has_none_of(fzero) );
	BOOST_TEST(  fzero.has_none_of(fA) );
	BOOST_TEST(  fzero.has_none_of(fAB) );

	BOOST_TEST(  fA.has_none_of(fzero) ); // has zero
	BOOST_TEST( !fA.has_none_of(fA) ); // has all
	BOOST_TEST( !fAB.has_none_of(fAB) ); // has all
	BOOST_TEST( !fAB.has_none_of(fABC) ); // has one
	BOOST_TEST(  fAB.has_none_of(fabc) ); // has none

	// individual bits
	BOOST_TEST( !fzero.has(Flags::A) );
	BOOST_TEST(  fA.has(Flags::A) );
	BOOST_TEST(  fAB.has(Flags::A) );
	BOOST_TEST( !fabc.has(Flags::A) );

	// emptyness
	BOOST_TEST(  fzero.empty() );
	BOOST_TEST( !fA.empty() );
	BOOST_TEST( !fABC.empty() );
}

BOOST_AUTO_TEST_SUITE_END()

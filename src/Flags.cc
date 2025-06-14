#include "Flags.hh"

#include "Logging.hh" // bugf
#include "String.hh"

Flags::
Flags(const char *str) : Flags(String(str)) {}

Flags::
Flags(const String& str) {
	for (const char *p = str.c_str(); *p; p++) {
		unsigned char c = *p;

		if (c == '|') {
			*this += Flags(String(p + 1));
			break;
		}

		if (isdigit(c)) {
			unsigned long number = 0;

			while (*p && isdigit(*p)) {
				number = number * 10 + *p - '0';
				p++;
			}

			// go back one, so loop can increment
			p--;

			*this += Flags(number);
			continue;
		}

		// c - 'A' is unsigned because we test that c >= 'A', silences warning
		if (c >= 'A' && c <= 'Z' && (c - 'A') < (unsigned char)bits.size()) {
			bits.set(c - 'A');
			continue;
		}

		// c - 'a' is unsigned because we test that c >= 'a', silences warning
		if (c >= 'a' && c <= 'z' && (c - 'a') < (unsigned char)bits.size() - 26) {
			bits.set((c - 'a') + 26);
			continue;
		}

		Logging::bugf("Flags::Flags(String): unknown symbol '%c' in string", *p);
		break;
	}	
}

const String Flags::
to_string() const {
	String buf;

	for (unsigned long i = 0; i < bits.size(); i++) {
		if (bits[i]) {
			if (i < 26)
				buf += 'A' + i;
			else {
				buf += 'a' + (i - 26);
			}
		}	
	}

	if (buf.empty())
		buf = '0';

	return buf;
}

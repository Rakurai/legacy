#include <set>
#include <vector>
#include "progs/Type.hh"
#include "progs/Prog.hh"
#include "progs/contexts/Context.hh"
#include "Logging.hh"
#include "file.hh"
#include "channels.hh"

namespace progs {

void read_from_file(FILE *fp, Vnum vnum, std::vector<Prog *>& progs, std::set<Type>& types) {
	while (true) {
		char letter = fread_letter(fp);

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		if (letter != '>') {
			if (letter != '|')
				ungetc(letter, fp);

			break;
		}

		int line_no = 0;

		try {
			progs::Prog *prog = new progs::Prog(fp, vnum, line_no);
			progs.push_back(prog);
			types.insert(prog->type);
		}
		catch (String e) {
			Logging::file_bug(fp, Format::format("progs::read_from_file: exception caught in reading prog on vnum %d, line %d:\n%s", vnum, line_no, e), 0);
			exit(1);
		}

	}
}

void debug(const contexts::Context& context, const String& message) {
	int line_no = context.current_line;
	String indent;
	for (int i = 0; i < context.current_depth; i++)
		indent += "  ";

	String buf = Format::format("%3d) %s%s%s{x",
		line_no, line_no == 0 ? "{Y" : "", indent, message.replace("$", "$$"));
	wiznet(buf, nullptr, nullptr, WIZ_PROGDEBUG, 0, 0);
}

} // namespace progs

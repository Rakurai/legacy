#include <set>
#include <vector>
#include "progs/Type.hh"
#include "progs/Prog.hh"
#include "Logging.hh"
#include "file.hh"

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

		try {
			progs::Prog *prog = new progs::Prog(fp, vnum);
			progs.push_back(prog);
			types.insert(prog->type);
		}
		catch (String e) {
			Logging::file_bug(fp, Format::format("progs::read_from_file: exception caught in reading prog on vnum %d:\n%s", vnum, e), 0);
			exit(1);
		}

	}
}

} // namespace progs

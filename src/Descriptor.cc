#include "Descriptor.hh"

#include <unistd.h>

#include "Format.hh"
#include "Logging.hh"

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool Descriptor::
write(const String& txt) {
	int iStart;
	int nWrite;
	int nBlock;

	int length = txt.size();

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);

		if ((nWrite = ::write(desc, txt.c_str() + iStart, nBlock)) < 0) {
			perror("Write_to_descriptor");
/* I don't know what this does exactly, but C++11 doesn't like it -- Montrey
			if (errno == EBADF) {
				char *nullptr = nullptr;

				if (*nullptr != '\0') abort();
			}
*/
			return FALSE;
		}
	}

	return TRUE;
}

bool Descriptor::
read() {
	unsigned int iStart;

	/* Hold horses if pending command already. */
	if (incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iStart = strlen(inbuf);

	if (iStart >= sizeof(inbuf) - 10) {
		Logging::logf("%s input overflow!", host);
		write("\n*** PUT A LID ON IT!!! ***\n");
		return FALSE;
	}

	/* Snarf input. */
	for (; ;) {
		int nRead;
		nRead = ::read(desc, inbuf + iStart,
		             sizeof(inbuf) - 10 - iStart);

		if (nRead > 0) {
			iStart += nRead;

			// retain compatibility with \r line endings
			if (inbuf[iStart - 1] == '\n' || inbuf[iStart - 1] == '\r')
				break;
		}
		else if (nRead == 0) {
			Logging::logf("EOF on read from host '%s'", host);
			return FALSE;
		}
		else if (errno == EWOULDBLOCK)
			break;
		else {
			perror("Read_from_descriptor");
			return FALSE;
		}
	}

	inbuf[iStart] = '\0';
	return TRUE;
}


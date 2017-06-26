#include "control/PlayerController.hh"

namespace control {

PlayerController::
~PlayerController() {
	close_socket(descriptor);
}

void PlayerController::
update() {
	// get text from desc, interpret, make char do stuff
}

} // namespace control

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(Descriptor *d)
{
	int i, j, k;

	/*
	 * Hold horses if pending command already.
	 */
	if (d->incomm[0] != '\0')
		return;

	/*
	 * Look for at least one new line.
	 */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (d->inbuf[i] == '\0')
			return;
	}

	/*
	 * Canonical input processing.
	 */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (k >= MAX_INPUT_LENGTH - 2) {
			d->write("Line too long.\n");

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++) {
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;
			}

			d->inbuf[i]   = '\n';
			d->inbuf[i + 1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if (k == 0)
		d->incomm[k++] = ' ';

	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */

	if (k > 1 || d->incomm[0] == '!') {
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
			d->repeat = 0;
		else {
			if (++d->repeat >= 25) {
				Format::sprintf(log_buf, "%s: input spamming!", d->host);
				Logging::log(log_buf);
				wiznet("And the spammer of the year is:  $N!!!",
				       d->character, nullptr, WIZ_SPAM, 0, GET_RANK(d->character));

				if (d->incomm[0] == '!')
					wiznet(d->inlast, d->character, nullptr, WIZ_SPAM, 0,
					       GET_RANK(d->character));
				else
					wiznet(d->incomm, d->character, nullptr, WIZ_SPAM, 0,
					       GET_RANK(d->character));

				d->repeat = 0;
				/*
                d->write("\n*** PUT A LID ON IT!!! ***\n");
                strcpy( d->incomm, "quit" );
				*/
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (d->incomm[0] == '!') {
		/* Allow commands to be appended to "!" commands.
		   This is done, only if we don't overflow the buffer.
		   -- Outsider
		*/
		if ((strlen(d->incomm) + strlen(d->inlast) + 16) < MAX_INPUT_LENGTH) {
			char temp_buffer[MAX_INPUT_LENGTH];
			/* new command will be old command + everything after the "!" */
			Format::sprintf(temp_buffer, "%s%s", d->inlast, & (d->incomm[1]));
			strcpy(d->incomm, temp_buffer);
		}
		else   /* message was too long, use last command */
			strcpy(d->incomm, d->inlast);
	}
	else
		strcpy(d->inlast, d->incomm);

	/*
	 * Shift the input buffer.
	 */
	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;

	for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
		;

	return;
}

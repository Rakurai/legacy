#pragma once

#include "protocol.h"

namespace Protocol {

	protocol_t * create() {
		return ProtocolCreate();
	}

	void destroy(protocol_t *p) {
		ProtocolDestroy(p);
	}

	void negotiate(Descriptor& d) {
		ProtocolNegotiate(&d.data);
	}

	void no_echo(Descriptor& d, bool abOn) {
		ProtocolNoEcho(&d.data, abOn);
	}

	void input(Descriptor& d, char *apData, int aSize, char *apOut) {
		ProtocolInput(&d.data, apData, aSize, apOut);
	}

	const char * output(Descriptor& d, const char *apData, int *apLength) {
		return ProtocolOutput(&d.data, apData, apLength);
	}
};

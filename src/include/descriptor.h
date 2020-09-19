#include "../deps/msdp/protocol.h"

// original Merc style descriptor internals for compatibility with the MSDP protocol snippet
struct descriptor_data
{
    bool_t fcommand;
    int outtop;

    protocol_t *pProtocol;
};

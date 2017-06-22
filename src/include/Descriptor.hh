#pragma once

#include "declare.hh"
#include "String.hh"
#include "memory.hh"
#include "Pooled.hh"

/*
 * Descriptor (channel) structure.
 */
class Descriptor :
public Pooled<Descriptor>
{
public:
    Descriptor() {}
    virtual ~Descriptor() {}

    Descriptor *        next = nullptr;
    Descriptor *        snoop_by = nullptr;
    Character *         character = nullptr;
    Character *         original = nullptr;
    long                hostaddr = 0;       /* numeric IP addr -- Elrac */
    String              host;           /* text addr */
    sh_int              descriptor = 0;
    sh_int              connected = CON_GET_NAME;
    bool                fcommand = FALSE;
    char                inbuf           [4 * MAX_INPUT_LENGTH] = {0};
    char                incomm          [MAX_INPUT_LENGTH] = {0};
    char                inlast          [MAX_INPUT_LENGTH] = {0};
    int                 repeat = 0;
    String              outbuf;
    int                 port = 0;
    int                 ip = 0;
    String              showstr_head;
    sh_int              timer = 0;

private:
    Descriptor(const Descriptor&);
    Descriptor& operator=(const Descriptor&);
};

extern Descriptor   *descriptor_list; // in comm.c

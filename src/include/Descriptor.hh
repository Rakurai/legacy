#pragma once

#include "String.hh"
#include "memory.hh" // MAX_INPUT_LENGTH
#include "Pooled.hh"

class Character;
namespace conn { struct State; }

/*
 * Descriptor (channel) structure.
 */
class Descriptor :
public Pooled<Descriptor>
{
public:
    Descriptor();
    virtual ~Descriptor() {}

    bool is_playing() const;

    Descriptor *        next = nullptr;
    Descriptor *        snoop_by = nullptr;
    Character *         character = nullptr;
    Character *         original = nullptr;
    long                hostaddr = 0;       /* numeric IP addr -- Elrac */
    String              host;           /* text addr */
    int              descriptor = 0;
    bool                fcommand = false;
    char                inbuf           [4 * MAX_INPUT_LENGTH] = {0};
    char                incomm          [MAX_INPUT_LENGTH] = {0};
    char                inlast          [MAX_INPUT_LENGTH] = {0};
    int                 repeat = 0;
    String              outbuf;
    int                 port = 0;
    int                 ip = 0;
    String              showstr_head;
    int              timer = 0;
    conn::State         *state;

private:
    Descriptor(const Descriptor&);
    Descriptor& operator=(const Descriptor&);
};

extern Descriptor   *descriptor_list; // in comm.c

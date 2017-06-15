#pragma once

#include "String.hh"

/*
 * Extra description data for a room or object.
 */
class ExtraDescr
{
public:
	ExtraDescr() {}
	virtual ~ExtraDescr() {}

    ExtraDescr *next = nullptr;     /* Next in list                     */
    bool valid = FALSE;
    String keyword;              /* Keyword in look/examine          */
    String description;          /* What to see                      */

private:
	ExtraDescr(const ExtraDescr&);
	ExtraDescr& operator=(const ExtraDescr&);
};

/* extra descr recycling */
ExtraDescr	*new_extra_descr args( (void) );
void	free_extra_descr args( (ExtraDescr *ed) );

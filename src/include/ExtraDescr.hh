#pragma once

#include "declare.hh"
#include "String.hh"
#include "Pooled.hh"

/*
 * Extra description data for a room or object.
 */
class ExtraDescr
// : public Pooled<ExtraDescr>
{
public:
	ExtraDescr() {}
	ExtraDescr(const String& k, const String& d) : keyword(k), description(d) {}
	ExtraDescr(const ExtraDescr& e) : keyword(e.keyword), description(e.description) {}
	virtual ~ExtraDescr() {}

    ExtraDescr *next = nullptr;     /* Next in list                     */
    String keyword;              /* Keyword in look/examine          */
    String description;          /* What to see                      */

private:
	ExtraDescr& operator=(const ExtraDescr&);
};

/* extra descr recycling */
ExtraDescr	*new_extra_descr args( (void) );
void	free_extra_descr args( (ExtraDescr *ed) );

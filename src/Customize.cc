#include "Customize.hh"
#include "merc.hh"

Customize::Customize() :
	skill_chosen(skill_table.size(), FALSE),
	group_chosen(group_table.size(), FALSE)
{}

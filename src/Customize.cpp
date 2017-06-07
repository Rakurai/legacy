#include "Customize.hpp"
#include "merc.h"

Customize::Customize() :
	skill_chosen(skill_table.size(), FALSE),
	group_chosen(group_table.size(), FALSE)
{}

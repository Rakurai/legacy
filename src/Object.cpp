#include "Object.hpp"
#include "affect_list.h"
#include "Affect.hpp"
#include "ExtraDescr.hpp"

Object::~Object() {
	// data wholly owned by this obj
	affect_clear_list(&this->affected);
	affect_clear_list(&this->gem_affected);

	ExtraDescr *ed, *ed_next;
	for (ed = this->extra_descr; ed != nullptr; ed = ed_next) {
		ed_next = ed->next;
		free_extra_descr(ed);
	}

	// deal with contents and gems at some point, for now extract_obj handles them
}

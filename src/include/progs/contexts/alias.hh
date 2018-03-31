#pragma once

#include "Format.hh"

namespace progs {
namespace contexts {

class Context;

template <typename T>
void get_alias(Context& context, const String& key, T* datap) {
	throw Format::format("progs::contexts::get_alias: unhandled type for key '%s'", key);
}

} // namespace contexts
} // namespace progs

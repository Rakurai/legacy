#pragma once

#include "String.hh"
#include "Flags.hh"
#include "Format.hh"

/**
 * @defgroup SQLStatusCodes Constants defining SQL status codes
 * @{
 */
constexpr int SQL_OK    = 0;
constexpr int SQL_ERROR = 1;
/** @} */

/* db functions */
void		db_open				();
void		db_close			();
void		db_error			(const String& func, const String& query);
int			db_next_row			();
int			db_get_column_int	(int index);
const Flags db_get_column_flags (int index);
const char * 
			db_get_column_str	(int index);
int			db_query			(const String& func, const String& query);
int			db_command			(const String& func, const String& query);
int			db_count			(const String& func, const String& query);
int			db_rows_affected	();
String      db_esc				(const String& str);

// variadic functions expanded at runtime, must be in header
template<class... Params>
int			db_queryf			(const String& func, const String& query, Params... params) {
	return db_query(func, Format::format(query, params...));
}

template<class... Params>
int			db_commandf			(const String& func, const String& query, Params... params) {
	return db_command(func, Format::format(query, params...));
}

template<class... Params>
int			db_countf			(const String& func, const String& query, Params... params) {
	return db_count(func, Format::format(query, params...));
}


#ifndef __SQL_H
#define __SQL_H

#include "Format.hpp"

#define SQL_OK		0
#define SQL_ERROR	1

/* db functions */
void		db_open				(void);
void		db_close			(void);
void		db_error			(const String& func);
int			db_next_row			();
int			db_get_column_int	(int index);
const char * 
			db_get_column_str	(int index);
int			db_query			(const String& func, const String& query);
int			db_command			(const String& func, const String& query);
int			db_count			(const String& func, const String& query);
int			db_rows_affected	();
String      db_esc				(const String& string);

// variadic functions expanded at runtime, must be in header
template<class... Params>
int			db_queryf			(const String& func, const String& query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	Format::sprintf(buf, query, params...);
	return db_query(func, buf);
}

template<class... Params>
int			db_commandf			(const String& func, const String& query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	Format::sprintf(buf, query, params...);
	return db_command(func, buf);
}

template<class... Params>
int			db_countf			(const String& func, const String& query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	Format::sprintf(buf, query, params...);
	return db_count(func, buf);
}

#endif /* __SQL_H */

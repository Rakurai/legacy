
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
int			db_query			(const char *func, const char *query);
int			db_command			(const char *func, const char *query);
int			db_count			(const char *func, const char *query);
int			db_rows_affected	();
String      db_esc				(const String& string);

// variadic functions expanded at runtime, must be in header
template<class... Params>
int			db_queryf			(const char *func, const char *query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	sprintf(buf, query, params...);
	return db_query(func, buf);
}

template<class... Params>
int			db_commandf			(const char *func, const char *query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	sprintf(buf, query, params...);
	return db_command(func, buf);
}

template<class... Params>
int			db_countf			(const char *func, const char *query, Params... params) {
	char buf[MAX_STRING_LENGTH*3];
	sprintf(buf, query, params...);
	return db_count(func, buf);
}

#endif /* __SQL_H */

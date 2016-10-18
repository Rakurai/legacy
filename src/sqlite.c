/**
 * mysql.c: MySQL wrapper functions
 *
 * Author: Jason Anderson (Montrey) <montrey@xenith.org>
 *
 */

#include <sqlite3.h>
#include "merc.h"
#include "db.h"
#include "sql.h"

sqlite3* _db = NULL;
sqlite3_stmt* _result = NULL;

void db_open()
{
	int error;

	if (_db)
		bugf("db_open: db is not NULL, opening anyway");

	error = sqlite3_open_v2(DB_FILE, &_db, SQLITE_OPEN_READWRITE, NULL);

	if (error != SQLITE_OK)
		db_error("db_open");
}

void db_close()
{
	if (_db != NULL) {
		sqlite3_close(_db);
		_db = NULL;
	}
}

void db_error(char *func)
{
	bugf("%s: %s", func, sqlite3_errmsg(_db));

	if (fBootDb) {
		db_close();
		exit(1);
	}
}

int db_next_row() {
	int ret = sqlite3_step(_result);

	switch (ret) {
		case SQLITE_ROW:
			return SQL_OK;
		case SQLITE_BUSY:
		case SQLITE_DONE:
		case SQLITE_ERROR:
		case SQLITE_MISUSE:
		default:
			break;
	}

	return SQL_ERROR;
}

int db_get_column_int(int index) {
	return sqlite3_column_int(_result, index);
}

const char * db_get_column_str(int index) {
	return (char *)sqlite3_column_text(_result, index);
}

/* perform a query, store and return a result */
int db_query(char *func, char *query)
{
	int error;

	if (_result != NULL) {
		// free result from last time
		sqlite3_finalize(_result);
		_result = NULL;
	}

	error = sqlite3_prepare_v2(_db, query, -1, &_result, NULL);

	if (error != SQLITE_OK) {
		db_error("db_query");
		return SQL_ERROR;
	}

	return SQL_OK;
}

/* format the args, send the query to db_query() */
int db_queryf(char *func, char *query, ...)
{
	char buf[MSL * 3];
	va_list args;
	va_start(args, query);
	vsnprintf(buf, MSL, query, args);
	va_end(args);
	return db_query(func, buf);
}

/* perform a query, no result, return success or not */
int db_command(char *func, char *query)
{
	int error = db_query(func, query);

	if (error == SQL_OK)
		error = db_next_row();

	return error;
}

/* format the args, perform a query, no result, return success or not */
int db_commandf(char *func, char *query, ...)
{
	char buf[MSL * 3];
	va_list args;
	va_start(args, query);
	vsnprintf(buf, MSL, query, args);
	va_end(args);
	return db_command(func, buf);
}

/* return a count.  note that this function takes a full query, including
   the SELECT COUNT(*) or COUNT(expr).  The only difference is that it
   does not return a result, just an integer.  Returns are 0 on no rows
   matching, a positive number for rows found, or -1 on error. */
int db_count(char *func, char *query)
{
	int error = db_command(func, query);

	if (error == SQL_OK)
		return db_get_column_int(0);

	return 0;
}

int db_countf(char *func, char *query, ...)
{
	char buf[MSL * 3];
	va_list args;
	va_start(args, query);
	vsnprintf(buf, MSL, query, args);
	va_end(args);
	return db_count(func, buf);
}

int db_rows_affected() {
	return sqlite3_changes(_db);
}

/* escapes a string for a mysql query, using the semiperm string list */
char *db_esc(char *string)
{
	char buf[3*MSL];
	int i = 0, j = 0;

	while (string[i] != '\0') {
		if (string[i] == '\'')
			buf[j++] = '\'';
// only for mysql compatibility?  don't use double quotes in queries,
// and don't double them here
//		else if (string[i] == '\"') 
//			buf[j++] = '\"';

		buf[j++] = string[i++];
	}

	buf[j] = '\0';
	return str_dup_semiperm(buf);
}

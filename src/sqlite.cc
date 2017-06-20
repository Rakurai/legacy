/**
 * mysql.c: MySQL wrapper functions
 *
 * Author: Jason Anderson (Montrey) <montrey@xenith.org>
 *
 */

#include <sqlite3.h>

#include "db.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "sql.hh"
#include "String.hh"

sqlite3* _db = nullptr;
sqlite3_stmt* _result = nullptr;

void db_open()
{
	int error;

	if (_db)
		Logging::bug("db_open: db is not nullptr, opening anyway", 0);

	error = sqlite3_open_v2(DB_FILE, &_db, SQLITE_OPEN_READWRITE, nullptr);

	if (error != SQLITE_OK)
		db_error("db_open");
}

void db_close()
{
	if (_db != nullptr) {
		sqlite3_close(_db);
		_db = nullptr;
	}
}

void db_error(const String& func)
{
	Logging::bugf("%s: %s", func, sqlite3_errmsg(_db));

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

const Flags db_get_column_flags(int index) {
	return Flags(sqlite3_column_int(_result, index));
}

const char * db_get_column_str(int index) {
	return (char *)sqlite3_column_text(_result, index);
}

/* perform a query, store and return a result */
int db_query(const String& func, const String& query)
{
	int error;

	if (_db == nullptr)
		db_open();

	if (_result != nullptr) {
		// free result from last time
		sqlite3_finalize(_result);
		_result = nullptr;
	}

	error = sqlite3_prepare_v2(_db, query.c_str(), -1, &_result, nullptr);

	if (error != SQLITE_OK) {
		db_error("db_query");
		return SQL_ERROR;
	}

	return SQL_OK;
}

/* perform a query, no result, return success or not */
int db_command(const String& func, const String& query)
{
	int error = db_query(func, query);

	if (error == SQL_OK)
		error = db_next_row();

	return error;
}

/* return a count.  note that this function takes a full query, including
   the SELECT COUNT(*) or COUNT(expr).  The only difference is that it
   does not return a result, just an integer.  Returns are 0 on no rows
   matching, a positive number for rows found, or -1 on error. */
int db_count(const String& func, const String& query)
{
	int error = db_command(func, query);

	if (error == SQL_OK)
		return db_get_column_int(0);

	return 0;
}

int db_rows_affected() {
	return sqlite3_changes(_db);
}

/* escapes a string for a mysql query */
String db_esc(const String& string)
{
	String buf;
	int i = 0;

	while (string[i] != '\0') {
		if (string[i] == '\'')
			buf += '\'';
// only for mysql compatibility?  don't use double quotes in queries,
// and don't double them here
//		else if (string[i] == '\"') 
//			buf[j++] = '\"';

		buf += string[i++];
	}

	return buf;
}

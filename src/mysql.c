/**
 * mysql.c: MySQL wrapper functions
 *
 * Author: Jason Anderson (Montrey) <montrey@xenith.org>
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "merc.h"
#include "db.h"
#include "include/mysql.h"

MYSQL *mysql_db;
char *DB_HOST = NULL;
char *DB_NAME = NULL;
char *DB_USER = NULL;
char *DB_PASS = NULL;

void db_open()
{
	if (mysql_db)
		bugf("db_open: mysql_db is not NULL, opening anyway");

	if ((mysql_db = mysql_init(mysql_db)) == NULL)
		db_error("db_open");
	else if (!mysql_real_connect(mysql_db, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, DB_SOCKET, DB_CLIENT_FLAG))
		db_error("db_open");
}

void db_close()
{
	if (mysql_db) {
		mysql_close(mysql_db);
		mysql_db = NULL;
	}
}

void db_error(char *func)
{
	bugf("%s: %s", func, mysql_error(mysql_db));

	if (fBootDb) {
		db_close();
		exit(1);
	}

//		bombout();
}

/* sends a query to the db.  the result set can be stored in a higher function */
int db_send_query(char *func, char *query)
{
	int i;

	if (mysql_db == NULL) {
		bugf("db_send_query(%s): mysql_db is NULL, reopening", func);
		db_open();
	}

	for (i = 1; i < 4; i++) {
		if (!mysql_real_query(mysql_db, query, strlen(query)))
			break;

		switch (mysql_errno(mysql_db)) {
		case CR_SERVER_GONE_ERROR:
		case CR_SERVER_LOST: {
				db_error(func);

				if (i == 3) {
					bugf("db_send_query(%s): abandoning query after 3 failed attempts", func);
					return 0;
				}

				bugf("db_send_query(%s): attempt (%d) to resend the query...", func, i);
				continue;
			}

		default:
			break;
		}
	}

	return 1;
}

/* perform a query, store and return a result */
MYSQL_RES *db_query(char *func, char *query)
{
	MYSQL_RES *result;

	if (!db_send_query(func, query))
		return NULL;

	if ((result = mysql_store_result(mysql_db)) == NULL) {
		bugf("db_query(%s): failed to store mysql result set", func);
		return NULL;
	}

	return result;
}

/* format the args, send the query to db_query() */
MYSQL_RES *db_queryf(char *func, char *query, ...)
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
	/* returns 0 on failure, 1 on success */
	return db_send_query(func, query);
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
	MYSQL_RES *result;
	MYSQL_ROW row;
	int count;

	if ((result = db_query(func, query)) == NULL)
		return -1;

	if ((row = mysql_fetch_row(result)) == NULL) {
		bugf("db_count(%s): fetched row is NULL on non-NULL result set", func);
		return -1;
	}

	if (row[0] == NULL || row[0][0] == '\0') {
		bugf("db_count(%s): NULL or empty string in first column of result set", func);
		return -1;
	}

	count = atoi(row[0]);
	mysql_free_result(result);
	return count;
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

/* escapes a string for a mysql query, using the semiperm string list */
char *db_esc(char *string)
{
	char buf[MSL];
	mysql_real_escape_string(mysql_db, buf, string, strlen(string));
	return str_dup_semiperm(buf);
}

/* simply sends a silent test query to the database, to keep our connection from timing out */
void db_update()
{
	if (mysql_ping(mysql_db) != 0)
		db_error("db_update");
}

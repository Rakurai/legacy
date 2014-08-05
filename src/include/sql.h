/**
 * mysql.c: MySQL wrapper header
 *
 * Author: Jason Anderson (Montrey) <montrey@xenith.org>
 *
 */

#ifndef __SQL_H
#define __SQL_H

#include <errmsg.h>
#include <mysql.h>


#define args(list) list

/* MySQL defines */
#define DB_PORT			0
#define DB_SOCKET		NULL
#define DB_CLIENT_FLAG	0

// filled by configuration loading
extern char *DB_HOST;
extern char *DB_NAME;
extern char *DB_USER;
extern char *DB_PASS;

extern	MYSQL *mysql_db;

/* db functions */
void		db_open				(void);
void		db_close			(void);
void		db_error			(char *func);
MYSQL_RES	*db_query			(char *func, char *query);
MYSQL_RES	*db_queryf			(char *func, char *query, ...);
int			db_command			(char *func, char *query);
int			db_commandf			(char *func, char *query, ...);
int			db_count			(char *func, char *query);
int			db_countf			(char *func, char *query, ...);
char		*db_esc				(char *string);
void		db_update			(void);

#endif /* __SQL_H */

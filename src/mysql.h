/**
 * mysql.c: MySQL wrapper header
 *
 * Author: Jason Anderson (Montrey) <montrey@xenith.org>
 *
 */

#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <stdio.h>
#ifndef __MYSQL_H
#define __MYSQL_H


#define args(list) list

/* MySQL defines */
#define DB_HOST			"localhost"
#define DB_PORT			0
#define DB_SOCKET		NULL
#define DB_CLIENT_FLAG	0
#define DB_NAME			"newlegacy"
#define DB_USER			"legacy"
#define DB_PASS			NULL

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

#endif /* __MYSQL_H */

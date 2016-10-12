
#ifndef __SQL_H
#define __SQL_H


#define SQL_OK		0
#define SQL_ERROR	1

/* db functions */
void		db_open				(void);
void		db_close			(void);
void		db_error			(char *func);
int			db_next_row			();
int			db_get_column_int	(int index);
const char * 
			db_get_column_str	(int index);
int			db_query			(char *func, char *query);
int			db_queryf			(char *func, char *query, ...);
int			db_command			(char *func, char *query);
int			db_commandf			(char *func, char *query, ...);
int			db_count			(char *func, char *query);
int			db_countf			(char *func, char *query, ...);
int			db_rows_affected	();
char		*db_esc				(char *string);

#endif /* __SQL_H */

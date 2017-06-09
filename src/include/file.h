#pragma once

/* file.c */
char	fread_letter		args((FILE *fp));
int	fread_number		args((FILE *fp));
const Flags	fread_flag		args((FILE *fp));
String	fread_string		args((FILE *fp, char to_char = '~'));
String	fread_string_eol	args((FILE *fp));
void	fread_to_eol		args((FILE *fp));
String	fread_word		args((FILE *fp));
void	fappend			args((const char *file, const char *str));

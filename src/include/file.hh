#pragma once

#include "Flags.hh"
#include "String.hh"
#include <cstdio>

/* file.c */
char	fread_letter(FILE *fp);
int	fread_number(FILE *fp);
const Flags	fread_flag(FILE *fp);
String	fread_string(FILE *fp, char to_char = '~');
String	fread_string_eol(FILE *fp);
void	fread_to_eol(FILE *fp);
String	fread_word(FILE *fp);
void	fappend(const String& file, const String& str);

const char    *dizzy_ctime(time_t *timep);
time_t  dizzy_scantime(const String& ctime);

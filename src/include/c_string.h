#pragma once

char *	str_dup_semiperm	(const String& string);
char *  str_dup         ( const char *str) ;
char *  str_dup         ( const String& str) ;
void    free_string     ( char *pstr ) ;
bool    str_cmp         ( const String& astr, const String& bstr) ;
bool    str_prefix      ( const String& astr, const String& bstr) ;
bool    str_prefix1     ( const String& astr, const String& bstr) ;
bool    str_infix       ( const String& astr, const String& bstr) ;
bool    str_suffix      ( const String& astr, const String& bstr) ;
const char *  smash_bracket   ( const String& str) ;
const char *  smash_tilde     ( const String& str);
const char *	ignore_apostrophe (const String& str);
void	strcut		(char *str, unsigned int length);
const char *	strcenter	(const String& string, int space);
const char *	strrpc		(const String& replace, const String& with, const String& in);
const char *    strins          (const String& string, const String& ins, int place);
const char *  center_string_in_whitespace (const String& string, int length);
bool    is_name          (const String& str, const String& namelist) ;
bool    is_exact_name    (const String& str, const String& namelist) ;
bool    is_exact_name_color    (const String& str, const String& namelist) ;
bool    note_is_name     (const String& str, const String& namelist) ;
const char *capitalize(const char *str);

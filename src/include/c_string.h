#pragma once

//const char *	ignore_apostrophe (const String& str);
//void	strcut		(char *str, unsigned int length);
const char *	strcenter	(const String& string, int space);
const char *    strins          (const String& string, const String& ins, int place);
const char *  center_string_in_whitespace (const String& string, int length);
bool    is_name          (const String& str, const String& namelist, bool exact = false) ;
bool    is_exact_name    (const String& str, const String& namelist) ;
bool    is_exact_name_color    (const String& str, const String& namelist) ;
bool    note_is_name     (const String& str, const String& namelist) ;
const char *capitalize(const char *str);
bool    has_slash       ( const char *str ) ;

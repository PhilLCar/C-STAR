#ifndef STR_UTILS
#define STR_UTILS

#include <diagnostic.h>
#include <stream.h>
#include <oop.h>

// A string of characters, with length
typedef struct string {
  char *content;
  int   length;
} String;

streamable(String, string);
buildable(String, string, const char *cstr);

// ==== OBJECT FUNCTIONS ==== //
// Adds string <b> after <a>, (destroys <b>)
String *concat(String *a, String *b);

// Appends char <c> after string <s>
String *append(String *s, char c);

// Prepends string <s> with char <c>
String *prepend(String *s, char c);

// Inserts char <c> at the specified <index> in string <s>
String *inject(String *s, int index, char c);

// RETURNS the substring of <s> from index <start> of <length> characters
String *substring(String *s, int start, int length);

// RETURNS a string without leading and trailing whitespaces from <s>
String *trim(String *s);

// RETURNS 1 if both <a> and <b> are equal, 0 otherwise
int     equals(String *a, String *b);

// RETURNS index if <a> contains <b>, -1 otherwise
int     contains(String *a, String *b);

// ==== STREAM FUNCTIONS ==== //
fromStream(String);

// RETURNS a StringStream on <s>
StringStream *ssopen(String *s);

// Closes the StringStream <ss>
void          ssclose(StringStream *ss);

// Gets the next character on <ss>
char          ssgetc(StringStream *ss);

// Ungets a character <c> on <ss>
void          ssungetc(char c, StringStream *ss);

#endif
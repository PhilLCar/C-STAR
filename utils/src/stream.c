#include <stream.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
char sgetc(Stream *s)
{
  return s->getc(s->stream);
}

////////////////////////////////////////////////////////////////////////////////
void sungetc(char c, Stream *s)
{
  s->ungetc(c, s->stream);
}

////////////////////////////////////////////////////////////////////////////////
void sclose(Stream *s)
{
  s->close(s->stream);
  free(s);
}


////////////////////////////////////////////////////////////////////////////////
String *sgetl(Stream *s)
{
  String *str = newString("");
	char    c;

	while((c = sgetc(s)) != '\n' && c != EOF) append(str, c);

	if (c == EOF && !str->length) {
		deleteString(&str);
	}

  return str;
}

////////////////////////////////////////////////////////////////////////////////
Array *sgeta(Stream *s)
{
	Array  *array = newArray(sizeof(String*));
	String *line;

	if (array) while ((line = sgetl(s))) {
		if (line->length) push(array, &line);
		else              break;
	}

	if (!array->size && !line) {
		deleteArray(&array);
	}

	return array;
}

////////////////////////////////////////////////////////////////////////////////
void sskipl(Stream *s)
{
	while (sgetc(s) != '\n');
}
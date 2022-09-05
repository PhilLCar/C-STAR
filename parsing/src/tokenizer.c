#include <tokenizer.h>

#include <stdlib.h>
#include <string.h>

#include <array.h>

// Skips a line on the stream
/******************************************************************************/
void _skipline(Stream *s) 
{
	while (sgetc(s) != '\n');
}

// Skips a line on the stream
/******************************************************************************/
Array *_getarray(Stream *s) 
{
	Array  *list = newArray(sizeof(String*));
	String *line;
	
	while (1) {
		line = sgetline(s);
		if (line && line->length) push(list, &line);
		else break;
	}
	
	if (!line) deleteArray(&list);

	return list;
}

////////////////////////////////////////////////////////////////////////////////
int constokenizer(struct tokenizer *ptr, Stream *stream)
{
	int success = 1;

	ptr->lookahead = 1;
	ptr->escape    = 0;

	_skipline(stream);
	{
		String *str = sgetline(stream);

		if (str->length) ptr->escape = str->content[0];
		deleteString(&str);
	}

	for (int i = 0; success && i < SINGLE_SIZE; i++) {
		_skipline(stream);
		ptr->single_list[i] = sgetline(stream);
		
		if (!ptr->single_list[i]) {
			success = 0;
			for (--i; i >= 0; --i) deleteString(&ptr->single_list[i]);
		}
	}

	for (int i = 0; success && i < MULTI_SIZE; i++) {
		Array *array;
		_skipline(stream);
		array = _getarray(stream);

		if (array) {
			for (int j = 0; j < array->size; j++) {
				int lookahead = (*(String**)at(array, i))->length;
				if (ptr->lookahead > lookahead) ptr->lookahead = lookahead;
			}
		} else {
			success = 0;
			for (int j = 0; j < SINGLE_SIZE; j++) deleteString(&ptr->single_list[j]);
			for (--i; i >= 0; --i) {
				Array *multi = ptr->multi_list[i];
				for (int j = 0; j < multi->size; j++) {
					deleteString(at(multi, j));
				}
				deleteArray(&multi);
			}
		}
	}

	sclose(stream);

	return success;
}

////////////////////////////////////////////////////////////////////////////////
void freetokenizer(struct tokenizer *ptr)
{
	for (int i = 0; i < SINGLE_SIZE; i++) deleteString(&ptr->single_list[i]);
	for (int i = 0; i < MULTI_SIZE;  i++) {
		Array *multi = ptr->multi_list[i];
		for (int j = 0; j < multi->size; j++) {
			deleteString(at(multi, j));
		}
		deleteArray(&multi);
	}
}

////////////////////////////////////////////////////////////////////////////////
Tokenizer *newTokenizer(Stream *stream)
{
	Tokenizer *tokenizer = malloc(sizeof(Tokenizer));

	if (tokenizer && !constokenizer) {
		free(tokenizer);
		tokenizer = NULL;
	}

	return tokenizer;
}

////////////////////////////////////////////////////////////////////////////////
void deleteTokenizer(Tokenizer **tokenizer)
{
	if (*tokenizer) {
		freetokenizer(*tokenizer);
		free(*tokenizer);
		*tokenizer = NULL;
	}
}

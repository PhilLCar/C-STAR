#include <parser.h>

#include <stdlib.h>
#include <string.h>

// Gets characters one by one from a line in the input file
char *readchar(FILE *fptr)
{
	char c;
	unsigned char ws_size = 0, ws_cap = 2;
	char *ws = NULL;

	if (fptr != NULL) {
		ws = malloc(ws_cap * sizeof(char));
		if (ws != NULL) {
			while ((c = fgetc(fptr)) != '\n') {
				if (c == EOF) goto ws_fail;
				ws[ws_size++] = c;
				if (ws_size >= ws_cap) {
					char *t = realloc(ws, (ws_cap *= 2) * sizeof(char));
					if (t == NULL) {
					ws_fail:
						free(ws);
						fprintf(stderr, "Memory allocation error! (Code: 0)\n");
						return NULL;
					}
					ws = t;
				}
			}
			ws[ws_size] = 0;
		}
	}
	return ws;
}

// Gets words one by one, each separated by a new-line in the input file
char **readline(FILE *fptr)
{
	char c;
	unsigned char bsa_size = 0, bsa_cap = 16;
	char **bsa = NULL;

	if (fptr != NULL) {
		bsa = malloc(bsa_cap * sizeof(char *));
		if (bsa != NULL) {
			memset(bsa, 0, bsa_cap * sizeof(char *));
			while ((c = fgetc(fptr)) != '\n') {
				if (c == EOF) goto bs_fail;
				unsigned char bs_size = 0, bs_cap = 2;
				char *bs = malloc(bs_cap * sizeof(char));
				if (bs != NULL) {
					bsa[bsa_size++] = bs;
					if (bsa_size >= bsa_cap) {
						char **t = realloc(bsa, (bsa_cap *= 2) * sizeof(char *));
						if (t == NULL) goto bs_fail; 
						bsa = t;
						memset(bsa + bsa_size, 0, (bsa_cap - bsa_size) * sizeof(char *));
					}
					do {
						if (c == EOF)
							goto bs_fail;
						bs[bs_size++] = c;
						if (bs_size >= bs_cap) {
							char *t = realloc(bs, (bs_cap *= 2) * sizeof(char));
							if (t == NULL) goto bs_fail;
							bs = t;
							bsa[bsa_size - 1] = bs;
						}
					} while ((c = fgetc(fptr)) != '\n');
					bs[bs_size] = 0;
				} else {
				bs_fail:
					for (int i = 0; i < bsa_cap; i++) {
						if (bsa[i]) free(bsa[i]);
						else break;
					}
					free(bsa);
					bsa = NULL;
					fprintf(stderr, "Memory allocation error! (Code: 1)\n");
					break;
				}
			}
		}
	}
	return bsa;
}

// Removes one line from parsing
void emptyline(FILE *fptr)
{
	while (fgetc(fptr) != '\n');
}

// Creates the parser object from a .prs file
Parser *newParser(char *filename)
{
	FILE *fptr = fopen(filename, "r");
	Parser *parser = malloc(sizeof(Parser));
	if (fptr != NULL && parser != NULL) {
		emptyline(fptr); // Whitespaces
		parser->whitespaces  = readchar(fptr);
		emptyline(fptr); // Escape chars
		parser->escapes      = readchar(fptr);
		emptyline(fptr); // Strings
		parser->strings      = readline(fptr);
		emptyline(fptr); // Chars
		parser->chars        = readline(fptr);
		emptyline(fptr); // Line comments
		parser->linecom      = readline(fptr);
		emptyline(fptr); // Multiline comments
		parser->multicom     = readline(fptr);
		emptyline(fptr); // Operators
		parser->operators    = readline(fptr);
		emptyline(fptr); // Delimiters
		parser->delimiters   = readline(fptr);
		emptyline(fptr); // Breaksymbols
		parser->breaksymbols = readline(fptr);
		emptyline(fptr); // Reserved
		parser->reserved     = readline(fptr);
		emptyline(fptr); // Constants
		parser->constants    = readline(fptr);
		if (parser->whitespaces  == NULL ||
			  parser->escapes      == NULL ||
			  parser->strings      == NULL ||
			  parser->chars        == NULL ||
			  parser->linecom      == NULL ||
			  parser->multicom     == NULL ||
			  parser->operators    == NULL ||
			  parser->delimiters   == NULL ||
			  parser->breaksymbols == NULL ||
			  parser->reserved     == NULL ||
				parser->constants    == NULL)
		{
			deleteParser(&parser);
		}
		else
		{
			int lookahead = 0;
			for (int i = 0; parser->strings[i]; i++) {
				int l = (int)strlen(parser->strings[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->chars[i]; i++) {
				int l = (int)strlen(parser->chars[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->linecom[i]; i++) {
				int l = (int)strlen(parser->linecom[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->multicom[i]; i++) {
				int l = (int)strlen(parser->multicom[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->operators[i]; i++) {
				int l = (int)strlen(parser->operators[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->delimiters[i]; i++) {
				int l = (int)strlen(parser->delimiters[i]);
				if (l > lookahead) lookahead = l;
			}
			for (int i = 0; parser->breaksymbols[i]; i++) {
				int l = (int)strlen(parser->breaksymbols[i]);
				if (l > lookahead) lookahead = l;
			}
			parser->lookahead = lookahead;
		}

		fclose(fptr);
		return parser;
	} else {
		if (parser) free(parser);
		if (fptr)   fclose(fptr);
	}

	return NULL;
}

void deleteParser(Parser **parser)
{
	if (*parser != NULL) {
		if ((*parser)->whitespaces  != NULL) free((*parser)->whitespaces);
		if ((*parser)->escapes      != NULL) free((*parser)->escapes);
		if ((*parser)->strings      != NULL) {
			for (int i = 0; (*parser)->strings[i]; i++)      free((*parser)->strings[i]);
			free((*parser)->strings);
		}
		if ((*parser)->chars       != NULL) {
			for (int i = 0; (*parser)->chars[i]; i++)        free((*parser)->chars[i]);
			free((*parser)->chars);
		}
		if ((*parser)->linecom      != NULL) {
			for (int i = 0; (*parser)->linecom[i]; i++)      free((*parser)->linecom[i]);
			free((*parser)->linecom);
		}
		if ((*parser)->multicom     != NULL) {
			for (int i = 0; (*parser)->multicom[i]; i++)     free((*parser)->multicom[i]);
			free((*parser)->multicom);
		}
		if ((*parser)->operators   != NULL) {
			for (int i = 0; (*parser)->operators[i]; i++)    free((*parser)->operators[i]);
			free((*parser)->operators);
		}
		if ((*parser)->delimiters   != NULL) {
			for (int i = 0; (*parser)->delimiters[i]; i++)   free((*parser)->delimiters[i]);
			free((*parser)->delimiters);
		}
		if ((*parser)->breaksymbols != NULL) {
			for (int i = 0; (*parser)->breaksymbols[i]; i++) free((*parser)->breaksymbols[i]);
			free((*parser)->breaksymbols);
		}
		if ((*parser)->reserved     != NULL) {
			for (int i = 0; (*parser)->reserved[i]; i++)     free((*parser)->reserved[i]);
			free((*parser)->reserved);
		}
		if ((*parser)->constants    != NULL) {
			for (int i = 0; (*parser)->constants[i]; i++)    free((*parser)->constants[i]);
			free((*parser)->constants);
		}
    free(*parser);
		*parser = NULL;
	}
}

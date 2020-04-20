#include <generic_parser.h>

// Gets characters one by one from a line in the input file
char *readchar(FILE *fptr)
{
	char c;
	unsigned char ws_size = 0, ws_cap = 2;
	char *ws = NULL;

	if (fptr != NULL)
	{
		ws = malloc(ws_cap * sizeof(char));
		if (ws != NULL)
		{
			while ((c = fgetc(fptr)) != '\n')
			{
				if (c == EOF)
					goto ws_fail;
				ws[ws_size++] = c;
				if (ws_size >= ws_cap)
				{
					char *t = realloc(ws, (ws_cap *= 2) * sizeof(char));
					if (t == NULL)
					{
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

	if (fptr != NULL)
	{
		bsa = malloc(bsa_cap * sizeof(char *));
		if (bsa != NULL)
		{
			memset(bsa, 0, bsa_cap * sizeof(char));
			while ((c = fgetc(fptr)) != '\n')
			{
				if (c == EOF)
					goto bs_fail;
				unsigned char bs_size = 0, bs_cap = 2;
				char *bs = malloc(bs_cap * sizeof(char));
				if (bs != NULL)
				{
					bsa[bsa_size++] = bs;
					if (bsa_size >= bsa_cap)
					{
						char **t = realloc(bsa, (bsa_cap *= 2) * sizeof(char *));
						if (t == NULL)
						{
							goto bs_fail;
						}
						bsa = t;
						memset(bsa + bsa_size, 0, (bsa_cap - bsa_size) * sizeof(char *));
					}
					do
					{
						if (c == EOF)
							goto bs_fail;
						bs[bs_size++] = c;
						if (bs_size >= bs_cap)
						{
							char *t = realloc(bs, (bs_cap *= 2) * sizeof(char));
							if (t == NULL)
							{
								goto bs_fail;
							}
							bs = t;
							bsa[bsa_size - 1] = bs;
						}
					} while ((c = fgetc(fptr)) != '\n');
					bs[bs_size] = 0;
				}
				else
				{
				bs_fail:
					for (int i = 0; i < bsa_cap; i++)
					{
						if (bsa[i])
						{
							free(bsa[i]);
						}
						else
							break;
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
	while (fgetc(fptr) != '\n')
		;
}

// Creates the parser object from a .prs file
Parser *newParser(char *filename)
{
	FILE *fptr = fopen(filename, "r");
	Parser *parser = malloc(sizeof(Parser));

	if (fptr != NULL && parser != NULL)
	{
		emptyline(fptr); // Whitespaces
		parser->whitespaces = readchar(fptr);
		emptyline(fptr); // Escape chars
		parser->escapes = readchar(fptr);
		emptyline(fptr); // Breaksymbols
		parser->breaksymbols = readline(fptr);

		if (parser->whitespaces == NULL ||
			parser->escapes == NULL ||
			parser->breaksymbols == NULL)
		{
			deleteParser(&parser);
		}
		else
		{
			int max_depth = 0;
			for (int i = 0; parser->breaksymbols[i]; i++)
			{
				int l = strlen(parser->breaksymbols[i]);
				if (l > max_depth)
				{
					max_depth = l;
				}
			}
			parser->max_depth = max_depth;
		}

		fclose(fptr);
		return parser;
	}

	return NULL;
}

void deleteParser(Parser **parser)
{
	if ((*parser)->whitespaces == NULL)
		free((*parser)->whitespaces);
	if ((*parser)->escapes == NULL)
		free((*parser)->escapes);
	if ((*parser)->breaksymbols == NULL)
		free((*parser)->breaksymbols);
	*parser = NULL;
}

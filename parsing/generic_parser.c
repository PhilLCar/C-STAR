#include "generic_parser.h"

// Gets characters one by one from a line in the input file
char *readchar(FILE *fptr) {
  char c;
  char ws_size = 0, ws_cap = 2;
  char *ws = NULL;
  
  if (fptr != NULL) {
    ws = malloc(ws_cap * sizeof(char));    
    if (ws != NULL) {
      while ((c = fgetc(fptr)) != '\n') {
	if (c == EOF) goto ws_fail;
	ws[ws_size++] = c;
	if (ws_size >= ws_cap) {	  
	  char *t = realloc(ws, (ws_cap *= 2));
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
char **readline(FILE *fptr) {
  char c;
  char bsa_size = 0, bsa_cap = 16;
  char **bsa = NULL;

  if (fptr != NULL) {
    bsa = malloc(bsa_cap * sizeof(char*));
    if (bsa != NULL) {
      memset(bsa, 0, sizeof(bsa));
      while ((c = fgetc(fptr)) != '\n') {
	if (c == EOF) goto bs_fail;
	char bs_size = 0, bs_cap = 2;
	char *bs = malloc(bs_cap * sizeof(char));
	if (bs != NULL) {
	  bsa[bsa_size++] = bs;
	  if (bsa_size >= bsa_cap) {
	    char **t = realloc(bsa, (bsa_cap *= 2));
	    if (t == NULL) {
	      goto bs_fail;
	    }
	    bsa = t;
	    memset(bsa + bsa_size, 0, (bsa_cap - bsa_size) * sizeof(char*));
	  }
	  do {
	    if (c == EOF) goto bs_fail;
	    bs[bs_size++] = c;
	    if (bs_size >= bs_cap) {
	      char *t = realloc(bs, (bs_cap *= 2));
	      if (t == NULL) {
	        goto bs_fail;
	      }
	      bs = t;
	      bsa[bsa_size - 1] = bs;
	    }
	  } while ((c = fgetc(fptr)) != '\n');
	  bs[bs_size] = 0;
	} else {
	bs_fail:
	  for (int i = 0; i < bsa_cap; i++) {
	    if (bsa[i]) {
	      free(bsa[i]);
	    } else break;
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
void emptyline(FILE *fptr) {
  while (fgetc(fptr) != '\n');
}

// Compares the string until one ends (will return true in that case)
int strcmps(char *s1, char *s2) {
  int i;
  for (i = 0;; i++) {
    if (s1[i] != s2[i]) {
      if (s1[i] && s2[i]) return 0;
      else break;
    }
    if (!s1[i] || !s2[i]) break;
  }
  return i;
}

// Extends a buffer by one char, reallocating if need be
int extend(char **buffer, int *size, int *cap, char c) {
  (*buffer)[(*size)++] = c;
  if (*size >= *cap) {
    char *t = realloc(*buffer, (*cap *= 2));
    if (t != NULL) {
      *buffer = t;
      memset(*buffer + *size, 0, (*cap - *size) * sizeof(char));
    }
    else return 0;
  }
  return 1;
}

// Creates the parser object from a .prs file
Parser *newParser(char *filename) {
  FILE *fptr = fopen(filename, "r");
  Parser *parser = malloc(sizeof(Parser));
  
  if (fptr != NULL && parser != NULL) {
    emptyline(fptr); // Whitespaces
    parser->whitespaces  = readchar(fptr);
    emptyline(fptr); // Escape chars
    parser->escape       = readchar(fptr);
    emptyline(fptr); // Breaksymbols
    parser->breaksymbols = readline(fptr);
    
    if (parser->whitespaces  == NULL ||
	parser->escape       == NULL ||
	parser->breaksymbols == NULL)
      {
	if (parser->whitespaces  == NULL) free(parser->whitespaces);
	if (parser->escape       == NULL) free(parser->escape);
	if (parser->breaksymbols == NULL) free(parser->breaksymbols);
	parser = NULL;
      }
    else {
      int max_depth = 0;
      for (int i = 0; parser->breaksymbols[i]; i++) {
	int l = strlen(parser->breaksymbols[i]);
	if (l > max_depth) {
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

// Returns the next symbol in the tracked file tf
int nextsymbol(TrackedFile *tf, Parser *parser, Symbol *symbol) {
  char  c;
  int   buf_size = 0, buf_cap = 2;
  char *buf = malloc(buf_cap * sizeof(char));
  int   new = 0;

  if (buf != NULL) {
    memset(buf, 0, buf_cap * sizeof(char));
    while(c = tfgetc(tf)) {
      if (c == EOF) break;
      int cmp, ws = 0;
      //////////////////////////////////////// NEW-LINE ////////////////////////////////////////
      if (c == '\n') {
	if (buf_size) {
	  tfungetc(tf, c);
	}
	else {
	  if(!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
	}
	break;
      }
      //////////////////////////////////////// WHITESPACE ////////////////////////////////////////
      for (int i = 0; parser->whitespaces[i]; i++) {
	if (c == parser->whitespaces[i]) ws = 1;
      }
      if (ws) {
	if(buf_size) {
	  break;
	}
	else continue;
      }
      //////////////////////////////////////// BREAKSYMBOLS ////////////////////////////////////////
      for (int i = 0; parser->breaksymbols[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->breaksymbols[i])) {
	  if (cmp > ws) {
	    ws = cmp;
	  }
	}
      }
      if (ws) {
        if (buf_size) {
	  tfungetc(tf, c);
	}
	else {
	  for (int i = 1;; i++) {
	    if(!extend(&buf, &buf_size, &buf_cap, c)) goto next_fail;
	    if (i == ws) break;
	    c = tfgetc(tf);
	  }
	}
	break;
      }
      //////////////////////////////////////// SYMBOL ////////////////////////////////////////
      if(!extend(&buf, &buf_size, &buf_cap, c)) {
      next_fail:
	free(buf);
	return 0;
      }
    }
    symbol->text = buf;
    symbol->line = tf->line;
    symbol->position = tf->position - buf_size;
    new = 1;
  }
  return new;
}

// Parses a file using the rules provided in parser
Symbol *parse(char *filename, Parser *parser) {
  TrackedFile *tf = tfopen(filename, parser->max_depth);
  int symbols_size = 0, symbols_cap = 1024;
  Symbol *symbols = NULL;
  if (tf != NULL) {
    symbols = malloc(symbols_cap * sizeof(Symbol));
    if (symbols != NULL) {
      memset(symbols, 0, symbols_cap * sizeof(Symbol));
      while (nextsymbol(tf, parser, &symbols[symbols_size])) {
	if (!symbols[symbols_size].text[0]) {
	  // END OF FILE
	  break;
	}
	printf("%s\n", symbols[symbols_size].text);
	if (++symbols_size == symbols_cap) {
	  Symbol *t = realloc(symbols, (symbols_cap *= 2));
	  if (t != NULL) {
	    symbols = t;
	    memset(symbols + symbols_size, 0, (symbols_cap - symbols_size) * sizeof(Symbol));
	  } else break;
	}
      }
    }
    tfclose(tf);
  }
  return symbols;
}

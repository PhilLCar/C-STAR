#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct parser {
  char   *whitespaces;
  char   *escape;
  char  **breaksymbols;
  char  **comments;
  char ***delimiters;
  char ***parentheses;
} Parser;

typedef struct symbol {
  char *text;
  int   length;
  int   line;
  int   position;
} Symbol;

void emptyline(FILE *fptr) {
  while (fgetc(fptr) != '\n');
}

char *chars(FILE *fptr) {
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

char **singles(FILE *fptr) {
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

char ***pairs(FILE *fptr) {
  char c;
  char pa_size = 0, pa_cap = 16;
  char ***pa = NULL;

  if (fptr != NULL) {
    pa = malloc(pa_cap * sizeof(char**));
    if (pa != NULL) {
      memset(pa, 0, sizeof(pa));
      while ((c = fgetc(fptr)) != '\n') {
	if (c == EOF) goto p_fail;
	char **p = malloc(2 * sizeof(char*));
	if (p != NULL) {
	  pa[pa_size++] = p;
	  if (pa_size >= pa_cap) {
	    char ***t = realloc(pa, (pa_cap *=2));
	    if (t == NULL) {
	      goto p_fail;
	    }
	    pa = t;
	    memset(pa + pa_size, 0, (pa_cap - pa_size) * sizeof(char*));
	  }
	  for (int i = 0; i < 2; i++) {
	    char pi_size = 0, pi_cap = 2;
	    char *pi = malloc(pi_cap * sizeof(char));
	    if (pi != NULL) {
	      if (c == EOF) goto p_fail;
	      p[i] = pi;
	      do {
		pi[pi_size++] = c;
		if (pi_size >= pi_cap) {
		  char *t = realloc(pi, (pi_cap *= 2));
		  if (t == NULL) {
		    goto p_fail;
		  }
		  pi = t;
		  p[i] = pi;
		}
	      } while ((c = fgetc(fptr)) != '\n');
	      pi[pi_size] = 0;
	      c = fgetc(fptr);
	    }
	  }
	} else {
	p_fail:
	  for (int i = 0; i < pa_cap; i++) {
	    if (pa[i]) {
	      if (pa[i][0]) free(pa[i][0]);
	      if (pa[i][1]) free(pa[i][1]);
	      free(pa[i]);
	    } else break;
	  }
	  free(pa);
	  pa = NULL;
	  fprintf(stderr, "Memory allocation error! (Code: 2)\n");
	  break;
	}
      }
    }
  }
  return pa;
}


Parser *newParser(char *filename) {
  FILE *fptr = fopen(filename, "r");
  Parser *parser = malloc(sizeof(Parser));
  
  if (fptr != NULL && parser != NULL) {
    emptyline(fptr); // Whitespaces
    parser->whitespaces  = chars(fptr);
    emptyline(fptr); // Escape chars
    parser->escape       = chars(fptr);
    emptyline(fptr); // Breaksymbols
    parser->breaksymbols = singles(fptr);
    emptyline(fptr); // Comment markers (to end of line)
    parser->comments     = singles(fptr);
    emptyline(fptr); // Delimiters (no stack)
    parser->delimiters   = pairs(fptr);
    emptyline(fptr); // Parentheses (with stack)
    parser->parentheses  = pairs(fptr);
    
    if (parser->whitespaces  == NULL ||
	parser->breaksymbols == NULL ||
	parser->delimiters   == NULL ||
	parser->parentheses  == NULL)
      {
	if (parser->whitespaces  == NULL) free(parser->whitespaces);
	if (parser->breaksymbols == NULL) free(parser->breaksymbols);
	if (parser->delimiters   == NULL) free(parser->delimiters);
	if (parser->parentheses  == NULL) free(parser->parentheses);
	parser = NULL;
      }
    
    fclose(fptr);
    return parser;
  }
  
  return NULL;
}

int nextsymbol(FILE *fptr, Symbol *symbol) {
  char c;
  while ((c = fgetc(fptr)) != '\n');
}

Symbol *parse(char *filename, Parser parser) {
  FILE *fptr = fopen(filename, "r");
  int symbols_size = 0, symbols_cap = 1024;
  Symbol *symbols = NULL;
  if (fptr != NULL) {
    symbols = malloc(symbols_cap * sizeof(Symbol));
    if (symbols != NULL) {
      
    }
    fclose(fptr);
  }
  return symbols;
}

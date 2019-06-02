#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct parser {
  char   *whitespaces;
  char   *escape;
  char  **breaksymbols;
  char  **comments;
  char ***delimiters;
  char ***brackets;
  int     max_depth;
  int    *stack;
  int     stack_cap;
  int     stack_size;
} Parser;

typedef struct symbol {
  char *text;
  int   length;
  int   line;
  int   position;
} Symbol;

typedef struct trackedFile {
  FILE *fptr;
  char *buffer;
  int   size;
  int   line;
  int   position;
} TrackedFile;

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
    parser->stack_size = 0;
    parser->stack_cap  = 16;
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
    emptyline(fptr); // Brackets (with stack)
    parser->brackets     = pairs(fptr);
    // Initialize stack
    parser->stack        = malloc(parser->stack_cap * sizeof(int));
    
    if (parser->whitespaces  == NULL ||
	parser->breaksymbols == NULL ||
	parser->delimiters   == NULL ||
	parser->brackets     == NULL ||
	parser->stack        == NULL)
      {
	if (parser->whitespaces  == NULL) free(parser->whitespaces);
	if (parser->breaksymbols == NULL) free(parser->breaksymbols);
	if (parser->delimiters   == NULL) free(parser->delimiters);
	if (parser->brackets     == NULL) free(parser->brackets);
	if (parser->stack        == NULL) free(parser->stack);
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
      for (int i = 0; parser->comments[i]; i++) {
	int l = strlen(parser->comments[i]);
	if (l > max_depth) {
	  max_depth = l;
	}
      }
      for (int i = 0; parser->delimiters[i]; i++) {
	for (int j = 0; j < 2; j++) {
	  int l = strlen(parser->delimiters[i][j]);
	  if (l > max_depth) {
	    max_depth = l;
	  }
	}
      }
      for (int i = 0; parser->brackets[i]; i++) {
	for (int j = 0; j < 2; j++) {
	  int l = strlen(parser->brackets[i][j]);
	  if (l > max_depth) {
	    max_depth = l;
	  }
	}
      }
      parser->max_depth = max_depth;
    }
    
    fclose(fptr);
    return parser;
  }
  
  return NULL;
}

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

void push(Parser *parser, int bracket_number) {
  parser->stack[parser->stack_size++] = bracket_number;
  if (parser->stack_size >= parser->stack_cap) {
    int *t = realloc(parser->stack, (parser->stack_cap *= 2));
    if (t != NULL) parser->stack = t;
  }
}

int pop(Parser *parser) {
  if (parser->stack_size > 0)
    return parser->stack[--parser->stack_size];
  else return -1;
}

int nextsymbol(TrackedFile *tf, Parser *parser, Symbol *symbol) {
  char  c;
  char  buf_size = 0, buf_cap = 32;
  char *buf = malloc(buf_cap * sizeof(char));
  int   tag = 0;

  if (buf != NULL && tag != NULL) {
    while(c = tfgetc(tf)) {
      if (c == EOF) break;
      int ws = 0;
      //////////////////////////////////////// WHITESPACE ////////////////////////////////////////
      for (int i = 0; parser->whitespaces[i]; i++) {
	if (c == parser->whitespaces[i]) ws = 1;
      }
      if (ws && !buf_size) continue;
      else if (ws && buf_size) break;
      //////////////////////////////////////// DELIMITERS ////////////////////////////////////////
      int cmp;
      for (int i = 0; parser->delimiters[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->delimiters[i][0])) {
	  if (cmp > ws) {
	    tag = i;
	    ws = cmp;
	  }
	}
      }
      if (ws && !buf_size) {
	// Check for delimiters
	// --> Check for escape chars
	// Return
      }
      else if (ws && buf_size) break;
      //////////////////////////////////////// COMMENTS ////////////////////////////////////////
      for (int i = 0; parser->comments[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->comments[i])) {
	  if (cmp > ws) {
	    tag = i;
	    ws = cmp;
	  }
	}
      }
      if (ws && !buf_size) {
	// Check for newline
	// Return
      }
      else if (ws && buf_size) break;
      //////////////////////////////////////// BREAKSYMBOLS ////////////////////////////////////////
      for (int i = 0; parser->breaksymbols[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->breaksymbols[i])) {
	  if (cmp > ws) {
	    tag = i;
	    ws = cmp;
	  }
	}
      }
      if (ws && !buf_size) {
	// Complete symbol parsing
	// Return
      }
      else if (ws && buf_size) break;
      //////////////////////////////////////// BRACKETS-O ////////////////////////////////////////
      for (int i = 0; parser->brackets[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->brackets[i][0])) {
	  if (cmp > ws) {
	    tag = i;
	    ws = cmp;
	  }
	}
      }
      if (ws && !buf_size) {
	// Complete bracket parsing
	// Return
      }
      //////////////////////////////////////// BRACKETS-C ////////////////////////////////////////
      for (int i = 0; parser->brackets[i]; i++) {
	if (cmp = strcmps(tf->buffer, parser->brackets[i][1])) {
	  if (cmp > ws) {
	    tag = i;
	    ws = cmp;
	  }
	}
      }
      if (ws && !buf_size) {
	// Complete bracket parsing
	// Return
      }
      else if (ws && buf_size) break;
      //////////////////////////////////////// SYMBOL ////////////////////////////////////////
      // Add char to buffer
    }
  }
  else return 0;
}

TrackedFile *tfopen(char *filename, int size) {
  FILE *fptr = fopen(filename, "r");
  char *buffer = malloc((size + 1) * sizeof(char));
  TrackedFile *tf = malloc(sizeof(TrackedFile));
  
  if (fptr != NULL && buffer != NULL && tf != NULL) {
    memset(buffer, 0, size + 1);
    tf->fptr     = fptr;
    tf->buffer   = buffer;
    tf->size     = size;
    tf->line     = 0;
    tf->position = 0;
  }
  if (buffer != NULL) free(buffer);
  if (tf != NULL) free(tf);
  return NULL;
}

void tfclose(TrackedFile *tf) {
  fclose(tf->fptr);
  free(tf->buffer);
}

char tfgetc(TrackedFile *tf) {
  if (tf->position || tf->line) {
    if (tf->buffer[0] == '\n') {
      tf->line++;
      tf->position = 0;
    } else tf->position++;
    for (int i = 0; i < tf->size; i++) {
      tf->buffer[i] = tf->buffer[i + 1];
    }
    tf->buffer[size] = fgetc(tf->fptr);
  } else {
    for (int i = 0; i <= tf->size; i++) {
      tf->buffer[i] = fgetc(tf->fptr);
    }
  }
  return tf->buffer[0];
}

void tfungetc(TrackedFile *tf, char c) {
  fungetc(tf->fptr, tf->buffer[tf->size]);
  for (int i = tf->size; i > 0; i--) {
    tf->buffer[i] = tf->buffer[i - 1];
  }
  tf->buffer[0] = c;
}

Symbol *parse(char *filename, Parser *parser) {
  TrackedFile *tf = tfopen(filename);
  int symbols_size = 0, symbols_cap = 1024;
  Symbol *symbols = NULL;
  if (tf != NULL) {
    symbols = malloc(symbols_cap * sizeof(Symbol));
    if (symbols != NULL) {
      
    }
    fclose(fptr);
  }
  return symbols;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct parser {
  char   *whitespaces;
  char  **breaksymbols;
  char ***delimiters;
  char ***parentheses;
} Parser;

char *whitespaces(FILE *fptr) {
  char c;
  char ws_size = 0, ws_cap = 2;
  char *ws = NULL;
  
  if (fptr != NULL) {
    ws = malloc(ws_cap * sizeof(char));    
    if (ws != NULL) {
      while ((c = fgetc(fptr)) != '\n') {
	ws[ws_size++] = c;
	if (ws_size >= ws_cap) {	  
	  char *t = realloc(ws, (ws_cap *= 2));
	  if (t == NULL) {
	    free(ws);
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

char **breaksymbols(FILE *fptr) {
  char c;
  char bsa_size = 0, bsa_cap = 16;
  char *bsa = NULL;

  if (fptr != NULL) {
    bsa = malloc(bsa_cap * sizeof(char*));
    if (bsa != NULL) {
      memset(bsa, 0, sizeof(bsa));
      while ((c = fgetc(fptr)) != '\n') {
	char bs_size = 0, bs_cap = 2;
	char *bs = malloc(bs_cap * sizeof(char));
	if (bs != NULL) {
	  bsa[bsa_size++] = bs;
	  if (bsa_size >= bsa_cap) {
	    char *t = realloc(bsa, (bsa_cap *= 2));
	    if (t == NULL) {
	      goto failure;
	    }
	    bsa = t;
	    memset(bsa + bsa_size, 0, bsa_size * sizeof(char*));
	  }
	  do {
	    bs[bs_size++] = c;
	    if (bs_size >= bs_cap) {
	      char *t = realloc(bs, (bs_cap *= 2));
	      if (t == NULL) {
	        goto failure;
	      }
	      bs = t;
	    }
	  } while ((c = fgetc(fptr)) != '\n');
	  bs[bs_size] = 0;
	} else {
	failure:
	  for (int i = 0; i < bsa_cap; i++) {
	    if (bsa[i]) {
	      free(bsa[i]);
	      break;
	    }
	  }
	  free(bsa);
	  bsa = NULL;
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
  char *pa = NULL;

  if (fptr != NULL) {
    pa = malloc(pa_cap * sizeof(char**));
    if (pa != NULL) {
      memset(pa, 0, sizeof(pa));
      
      while ((c = fgetc(fptr)) != '\n') {
	
      }
    }
  }
  return bsa;
}


newParser *fromFile(char *filename) {
  FILE *fptr = fopen(filename, "r");
  
  return NULL;
}

int parse(char *filename, Parser parser) {
  return 0;
}

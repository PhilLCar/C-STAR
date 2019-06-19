#ifndef ERROR_PARSING
#define ERROR_PARSING

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <terminal.h>
#include "symbol.h"

#define CONTEXT_LENGTH 35

void printerror(char*, char*, Symbol*);
void printwarning(char*, char*, Symbol*);
void printsuggest(char*, char*);

#endif

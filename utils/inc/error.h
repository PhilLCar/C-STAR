#ifndef ERROR_PARSING
#define ERROR_PARSING

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <terminal.h>
#include <symbol.h>

#define CONTEXT_LENGTH 35

typedef enum messagetype {
  INFO = 0,
  DEBUG,
  WARNING,
  ERROR
} MessageType;

void printnodemessage(MessageType, char*, char*, char**, char*);
void printsymbolmessage(MessageType, char*, char**, char*, Symbol*);
void printfilemessage(MessageType, char*, char**, char*);
void printmessage(MessageType, char*);
void printsuggest(char*, char*);

#endif

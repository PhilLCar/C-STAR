#include <preprocessor.h>

char *filenamewoext(char *filename)
{
  int   len = strlen(filename);
  char *woext = NULL;
  for (int i = len - 1; i >= 0; i--) {
    if (filename[i] == '.') {
      woext = malloc((i + 1) * sizeof(char));
      if (woext) {
        for (int j = 0; j < i; j++) {
          woext[j] = filename[j];
        }
        woext[i] = 0;
      }
      break;
    }
  }
  if (!woext) {
    woext = malloc((len + 1) * sizeof(char));
    if (woext) {
      memcpy(woext, filename, (len + 1) * sizeof(char));
    }
  }
  return woext;
}

char *fileext(char *filename) {
  char *ext = filename;
  while (ext[0] && ext[0] != '.') ext++;
  return ext;
}

Symbol *ppconsume(SymbolStream *ss, FILE *output) {
  char c;
  while ((c = tfgetc(ss->tfptr)) != EOF) {
    for (int i = 0; ss->parser->whitespaces[i]; i++) {
      if (c == ss->parser->whitespaces[i]) {
        fputc(c, output);
        c = 0;
        break;
      }
    }
    if (c) {
      tfungetc(ss->tfptr, c);
      break;
    }
  }
  return ssgets(ss);
}

int preprocessfile(char *filename, Array *incpath, Array *trace, PPEnv *ppenv, int search)
{
  if (trace->size >= INCLUDE_MAX_DEPTH) {
    printfilemessage(ERRLVL_ERROR, trace, "Reache inclusion maximum depth, check for recursive incldues!");
    return 0;
  }

  SymbolStream *ss      = ssopen(filename, ppenv->parser);
  Symbol       *s;
  char          error[256];
  char         *ext     = fileext(filename);
  //int           cmode   = 0;
  int           valid   = 1;

  if (!ss && search && filename[0] != '/') {
    for (int i = 0; i < incpath->size; i++) {
      char fnwpath[INCLUDE_MAX_FILE_LENGTH];
      sprintf(fnwpath, "%s/%s", *(char**)at(incpath, i), filename);
      ss = ssopen(fnwpath, ppenv->parser);
      if (ss) break;
    }
  }
  if (ss) push(trace, &ss->filename);
  else    valid = 0;

  if (valid) {
    if (!strcmp(ext, ".csr") || !strcmp(ext, ".hsr")) {
      // C* MODE
    } else if (!strcmp(ext, ".c") || !strcmp(ext, ".h")) {
      // C MODE
      //cmode = 1;
    } else {
      // UNKOWN FILE FORMAT -- ERROR
      sprintf(error, "The extension '%s' is unknown the the C* preprocessor!", ext);
      printfilemessage(ERRLVL_ERROR, trace, error);
      valid = 0;
    }
  }

  while (valid && !(s = ppconsume(ss, ppenv->output))->eof) {
    if (!strcmp(s->text, "#include")) {
      char fnwext[INCLUDE_MAX_FILE_LENGTH];
      s = ssgets(ss);
      if (s->type == SYMBOL_STRING) {
        // local include
        if (fileext(s->text)[0]) sprintf(fnwext, "%s", s->text);
        else                     sprintf(fnwext, "%s.hsr", s->text);
        if (strlen(fnwext) > INCLUDE_MAX_FILE_LENGTH) {
          sprintf(error, "Included file's name is too big!");
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        } else if (!preprocessfile(fnwext, incpath, trace, ppenv, 0)) {
          sprintf(error, "Error including file '%s'!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      } else if (!strcmp(s->text, "<")) {
        // native include
        s = ssgets(ss);
        if (fileext(s->text)[0]) sprintf(fnwext, "%s", s->text);
        else                     sprintf(fnwext, "%s.hsr", s->text);
        if (!preprocessfile(fnwext, incpath, trace, ppenv, 1)) {
          sprintf(error, "Error including file '%s'!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
        if (valid) {
          s = ssgets(ss);
          if (strcmp(s->text, ">")) {
            sprintf(error, "Expected '>' or '\"' got '%s' instead!", s->text);
            printsymbolmessage(ERRLVL_ERROR, trace, s, error);
            valid = 0;
          }
        }
      } else {
        sprintf(error, "Expected '<' or '\"' got '%s' instead!", s->text);
        printsymbolmessage(ERRLVL_ERROR, trace, s, error);
        valid = 0;
      }
      if (valid) {
        s = ssgets(ss);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    } else if (!strcmp(s->text, "#define")) {

    } else if (!strcmp(s->text, "#undef")) {

    } else if (!strcmp(s->text, "#ifdef")) {

    } else if (!strcmp(s->text, "#ifndef")) {

    } else if (!strcmp(s->text, "#elif")) {

    } else if (!strcmp(s->text, "#else")) {

    } else if (!strcmp(s->text, "#endif")) {

    } else if (!strcmp(s->text, "#warning")) {

    } else if (!strcmp(s->text, "#error")) {

    } else if (!strcmp(s->text, "#pragma")) {

    } else {
      while (!s->eof && s->text[0] != '\n') {
        for (int i = 0; s->text[i]; i++) fputc(s->text[i], ppenv->output);
        s = ppconsume(ss, ppenv->output);
      }
      fputc('\n', ppenv->output);
    }
  }

  if (ss) {
    pop(trace);
    ssclose(ss);
  }
  return valid;
}

void preprocess(char *filename, Array *incpath)
{
  char  ppfile[INCLUDE_MAX_FILE_LENGTH];
  char  metafile[INCLUDE_MAX_FILE_LENGTH];
  char *woext = filenamewoext(filename);
  sprintf(ppfile,   "%s.psr", woext);
  sprintf(metafile, "%s.msr", woext);
  free(woext);
  //////////////////////////////////////////
  Parser       *parser   = newParser("parsing/prs/csr.prs");
  FILE         *output   = fopen(ppfile,   "w+");
  FILE         *metadata = fopen(metafile, "w+");
  Array        *env      = newArray(sizeof(PPVar));
  Array        *trace    = newArray(sizeof(char*));
  PPEnv         ppenv;

  ppenv.output     = output;
  ppenv.metadata   = metadata;
  ppenv.parser     = parser;
  ppenv.env        = env;

  preprocessfile(filename, incpath, trace, &ppenv, 0);

  if (parser)   deleteParser(&parser);
  if (output)   fclose(output);
  if (metadata) fclose(metadata);
  if (env)      deleteArray(&env);
  if (trace)    deleteArray(&trace);
}
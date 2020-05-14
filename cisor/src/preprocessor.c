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
      tfungetc(c, ss->tfptr);
      break;
    }
  }
  return ssgets(ss);
}

Symbol *ppread(SymbolStream *ss, String *str) {
  char c;
  Symbol *s = NULL;
  if (tfgetc(ss->tfptr) != '\n') {
    while ((c = tfgetc(ss->tfptr)) != EOF && c != '\n') {
      if (c == '\\') {
        s = ssgets(ss);
        if (s->text[0] != '\n') {
          break;
        }
        s = NULL;
      }
      append(str, c);
    }
  }
  return s;
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
  int           control = 0;
  int           ignore  = 0;

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
    /////////////////////////////////////////////////////////////////////////////////////
    if (!strcmp(s->text, "#include") && !ignore) {
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
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#define") && !ignore) {
      Macro   m;
      String *str = newString("");
      s = ssgets(ss);
      m.name = malloc(strlen(s->text) + 1);
      sprintf(m.name, "%s", s->text);
      s = ppread(ss, str);
      if (s) printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
      m.value = str->content;
      str->content = NULL;
      deleteString(&str);
      push(ppenv->env, &m);
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#undef") && !ignore) {
      s = ssgets(ss);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to undefine, got nothing");
        valid = 0;
      }
      if (valid) {
        int undef = 0;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name)) {
            undef = 1;
            rem(ppenv->env, i);
            break;
          }
        }
        if (!undef) {
          sprintf(error, "'%s' is not defined...", s->text);
          printsymbolmessage(ERRLVL_WARNING, trace, s, error);
        }
        s = ssgets(ss);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#ifdef")) {
      s = ssgets(ss);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to verify, got nothing");
        valid = 0;
      }
      if (valid) {
        int def = 0;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name)) {
            def = 1;
            break;
          }
        }
        if (def) {
          control++;
        }
        s = ssgets(ss);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#ifndef")) {
      s = ssgets(ss);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to verify, got nothing");
        valid = 0;
      }
      if (valid) {
        int undef = 1;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name)) {
            undef = 0;
            break;
          }
        }
        if (undef) {
          control++;
        }
        s = ssgets(ss);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#if")) {
      ASTNode *ast     = newASTNode(NULL, NULL);
      while (!(s = ssgets(ss))->eof) {
        if (s->text[0] == '\n') break;
        if (s->text[0] == '\\') {
          s = ssgets(ss);
          if (s->text[0] != '\n') {
            valid = 0;
            break;
          }
          continue;
        }
        astnewsymbol(ast, ppenv->tree, ASTFLAGS_NONE, s);
        if (ast->status == STATUS_FAILED) {
          printsymbolmessage(ERRLVL_ERROR, trace, s, "Badely formatted expression!");
          valid = 0;
          break;
        }
      }
      deleteAST(&ast);
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#elif")) {

    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#else")) {

    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#endif")) {

    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#warning") && !ignore) {
      String *str = newString("");
      s = ppread(ss, str);
      if (s) printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
      else printsymbolmessage(ERRLVL_WARNING, trace, s, str->content);
      deleteString(&str);
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#error") && !ignore) {
      String *str = newString("");
      s = ppread(ss, str);
      if (s) printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
      else printsymbolmessage(ERRLVL_ERROR, trace, s, str->content);
      deleteString(&str);
      valid = 0;
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#pragma") && !ignore) {
      // UNIMPLEMENTED
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!ignore) {
      while (!s->eof && s->text[0] != '\n') {
        int expanded = 0;
        if (s->open) for (int i = 0; s->open[i]; i++)   fputc(s->open[i],  ppenv->output);
        else {
          for (int i = 0; i < ppenv->env->size; i++) {
            Macro *m   = at(ppenv->env, i);
            int    len = strlen(m->value);
            if (!strcmp(s->text, m->name)) {
              expanded = 1;
              for (int j = 0; j < len; j++)             fputc(m->value[j], ppenv->output);
              break;
            }
          }
        }
        if (!expanded) for (int i = 0; s->text[i]; i++) fputc(s->text[i],  ppenv->output);
        if (s->close) for (int i = 0; s->close[i]; i++) fputc(s->close[i], ppenv->output);
        s = ppconsume(ss, ppenv->output);
      }
      fputc('\n', ppenv->output);
    } else {
      char c;
      while ((c = tfgetc(ss->tfptr)) != '#' && c != EOF);
      tfungetc('#', ss->tfptr);
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
  Array        *env      = newArray(sizeof(Macro));
  Array        *stack    = newArray(sizeof(int));
  BNFNode      *tree     = parsebnf("parsing/bnf/preprocessor.bnf");
  Array        *trace    = newArray(sizeof(char*));
  PPEnv         ppenv;

  ppenv.output     = output;
  ppenv.metadata   = metadata;
  ppenv.parser     = parser;
  ppenv.env        = env;
  ppenv.stack      = stack;
  ppenv.tree       = *(BNFNode**)at(tree->content, 0);

  preprocessfile(filename, incpath, trace, &ppenv, 0);

  for (int i = 0; i < env->size; i++) {
    Macro *m = at(env, i);
    free(m->name);
    free(m->value);
  }
  if (parser)   deleteParser(&parser);
  if (output)   fclose(output);
  if (metadata) fclose(metadata);
  if (env)      deleteArray(&env);
  if (stack)    deleteArray(&stack);
  if (tree)     deleteBNFTree(&tree);
  if (trace)    deleteArray(&trace);
}
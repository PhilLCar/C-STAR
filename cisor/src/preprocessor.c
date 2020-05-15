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
  Symbol *s     = NULL;
  char    c;

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
  trim(str);
  return s;
}

int preprocessfile(char *filename, Array *incpath, Array *trace, PPEnv *ppenv, int search)
{
  if (trace->size >= INCLUDE_MAX_DEPTH) {
    printfilemessage(ERRLVL_ERROR, trace, "Reached inclusion maximum depth, check for recursive incldues!");
    return 0;
  }

  SymbolStream *ss      = ssopen(filename, ppenv->parser);
  Symbol       *s;
  char          error[1024];
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
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
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
        while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#define") && !ignore) {
      Macro   m;
      char    c;
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      m.filename = newString(filename);
      m.name     = newString(s->text);
      m.value    = newString("");
      m.params   = newArray(sizeof(String*));
      m.line     = s->line;
      m.position = s->position;

      if (m.name->length > MACRO_NAME_MAX_LENGTH) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Macro name exceeds maximum length!");
        freemacro(&m);
        valid = 0;
      }
      if (valid) {
        int def = 0;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *c = at(ppenv->env, i);
          if (equals(m.name, c->name)) {
            def = 1;
            freemacro(c);
            set(ppenv->env, i, &m);
            break;
          }
        }
        if (!def) push(ppenv->env, &m);
        else {
          sprintf(error, "Macro '%s' is already defined, the previous definition was overwritten...", m.name->content);
          printsymbolmessage(ERRLVL_WARNING, trace, s, error);
        }
        c = tfgetc(ss->tfptr);
        if (c == '(') {
          while (!(s = ssgets(ss))->eof) {
            if (!strcmp(s->text, ")")) break;
            if (!strcmp(s->text, ",")) continue;
            String *t = newString(s->text);
            push(m.params, &t);
          }
        }
        if (c != '\n' && c != EOF) {
          s = ppread(ss, m.value);
          if (s) {
            printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
            valid = 0;
          }
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#undef") && !ignore) {
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to undefine, got nothing");
        valid = 0;
      }
      if (valid) {
        int undef = 0;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name->content)) {
            undef = 1;
            freemacro(m);
            rem(ppenv->env, i);
            break;
          }
        }
        if (!undef) {
          sprintf(error, "'%s' is not defined...", s->text);
          printsymbolmessage(ERRLVL_WARNING, trace, s, error);
        }
        while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#ifdef")) {
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to verify, got nothing");
        valid = 0;
      }
      if (valid) {
        int def = 0;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name->content)) {
            def = 1;
            break;
          }
        }
        if (def) {
          control++;
        }
        while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
        if (s->text[0] && s->text[0] != '\n') {
          sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
          printsymbolmessage(ERRLVL_ERROR, trace, s, error);
          valid = 0;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#ifndef")) {
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      if (s->eof || !strcmp(s->text, "\n")) {
        printsymbolmessage(ERRLVL_ERROR, trace, s, "Expected a macro to verify, got nothing");
        valid = 0;
      }
      if (valid) {
        int undef = 1;
        for (int i = 0; i < ppenv->env->size; i++) {
          Macro *m = at(ppenv->env, i);
          if (!strcmp(s->text, m->name->content)) {
            undef = 0;
            break;
          }
        }
        if (undef) {
          control++;
        }
        while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
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
      Symbol *sym = newSymbol(s);
      char    c;
      if ((c = tfgetc(ss->tfptr)) != '\n' && c != EOF) {
        s = ppread(ss, str);
        if (s) {
          printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
          valid = 0;
        }
      } else {
        concat(str, newString("No message..."));
      }
      printsymbolmessage(ERRLVL_WARNING, trace, sym, str->content);
      deleteString(&str);
      deleteSymbol(&sym);
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#error") && !ignore) {
      String *str = newString("");
      Symbol *sym = newSymbol(s);
      char    c;
      if ((c = tfgetc(ss->tfptr)) != '\n' && c != EOF) {
        s = ppread(ss, str);
        if (s) printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
      } else {
        concat(str, newString("No message..."));
      }
      printsymbolmessage(ERRLVL_ERROR, trace, sym, str->content);
      deleteString(&str);
      deleteSymbol(&sym);
      valid = 0;
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#pragma") && !ignore) {
      // UNIMPLEMENTED
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!ignore) {
      while (!s->eof && s->text[0] != '\n') {
        if (s->open) for (int i = 0; s->open[i]; i++) fputc(s->open[i], ppenv->output);
        if (s->type == SYMBOL_VARIABLE) {
          String *str = newString(s->text);
          char c = tfgetc(ss->tfptr);
          if (c == '(') {
            int p = 1;
            do {
              append(str, c);
              c = tfgetc(ss->tfptr);
              if (c == '(')      p++;
              else if (c == ')') p--;
            } while (p > 0);
            append(str, c);
          } else tfungetc(c, ss->tfptr);
          Expansion *e = newExpansion();
          macroexpand(ppenv->env, ppenv->parser, str, e, NULL);
          switch (e->invalid) {
            case MACRO_ERROR_MAX_DEPTH:
              printmacromessage(ERRLVL_ERROR, trace, e->hist, "Reached maximum expansion depth!");
              valid = 0;
              break;
            default:
              for (int i = 0; i < e->value->length; i++) fputc(e->value->content[i], ppenv->output);
              break;
          }
          deleteExpansion(&e);
          deleteString(&str);
        }
        else for (int i = 0; s->text[i]; i++)           fputc(s->text[i],      ppenv->output);
        if (s->close) for (int i = 0; s->close[i]; i++) fputc(s->close[i],     ppenv->output);
        s = ppconsume(ss, ppenv->output);
      }
      fputc('\n', ppenv->output);
    } else {
      char c;
      while ((c = tfgetc(ss->tfptr)) != '#' && c != EOF);
      if (c != EOF) tfungetc('#', ss->tfptr);
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
    freemacro(at(env, i));
  }
  if (parser)   deleteParser(&parser);
  if (output)   fclose(output);
  if (metadata) fclose(metadata);
  if (env)      deleteArray(&env);
  if (stack)    deleteArray(&stack);
  if (tree)     deleteBNFTree(&tree);
  if (trace)    deleteArray(&trace);
}
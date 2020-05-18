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

PPResult ppeval(ASTNode *ast, PPEnv *ppenv) {
  PPResult result;
  if (!strcmp(ast->name->content, "operation4") ||
      !strcmp(ast->name->content, "operation3") ||
      !strcmp(ast->name->content, "operation2") ||
      !strcmp(ast->name->content, "operation1")) {
    PPResult  a  = ppeval(*(ASTNode**)at(ast->subnodes, 0), ppenv);
    PPResult  b  = ppeval(*(ASTNode**)at(ast->subnodes, 2), ppenv);
    String   *op = (*(ASTNode**)at(ast->subnodes, 1))->value;
    
    if (a.type == PPTYPE_ERROR) {
      result.type  = PPTYPE_ERROR;
      result.value = a.value;
    } else if (b.type == PPTYPE_ERROR) {
      result.type  = PPTYPE_ERROR;
      result.value = b.value;
    } else if (!strcmp(op->content, "^")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value ^ (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Bitwise operator '^' only applies to integer values!";
      }
    } else if (!strcmp(op->content, "&")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value & (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Bitwise operator '&' only applies to integer values!";
      }
    } else if (!strcmp(op->content, "|")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value | (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Bitwise operator '|' only applies to integer values!";
      }
    } else if (!strcmp(op->content, "*")) {
      // add decimal support!
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value * (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Operator '*' only applies to number values!";
      }
    } else if (!strcmp(op->content, "/")) {
      // add decimal support!
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value / (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Operator '/' only applies to number values!";
      }
    } else if (!strcmp(op->content, "%")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value % (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Operator '%' only applies to integer values!";
      }
    } else if (!strcmp(op->content, "+")) {
      // add decimal support!
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value + (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String concat
        result.value = "Operator '+' only applies to number values!";
      }
    } else if (!strcmp(op->content, "-")) {
      // add decimal support!
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)((long)a.value - (long)b.value);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String remove
        result.value = "Operator '-' only applies to number values!";
      }
    } else if (!strcmp(op->content, "==")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value == (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) == 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '==' can only compare values of the same type";
      }
    } else if (!strcmp(op->content, "!=")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value != (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) != 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '!=' can only compare values of the same type";
      }
    } else if (!strcmp(op->content, "<=")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value <= (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) <= 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '<=' can only compare values of the same type";
      }
    } else if (!strcmp(op->content, "<")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value < (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) < 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '<' can only compare values of the same type";
      }
    } else if (!strcmp(op->content, ">=")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value >= (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) >= 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '>=' can only compare values of the same type";
      }
    } else if (!strcmp(op->content, ">")) {
      if (a.type == PPTYPE_INT && b.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)((long)a.value > (long)b.value);
      } else if (a.type == PPTYPE_STRING && b.type == PPTYPE_STRING) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(long)(strcmp(a.value, b.value) > 0);
      } else {
        result.type  = PPTYPE_ERROR;
        /// TODO: String length compare
        result.value = "Operator '>' can only compare values of the same type";
      }
    }
  } else if (!strcmp(ast->name->content, "operation5")) {
    PPResult a = ppeval(*(ASTNode**)at(ast->subnodes, 0), ppenv);
    if (a.type == PPTYPE_ERROR) {
      result.type  = PPTYPE_ERROR;
      result.value = a.value;
    } else if (a.type == PPTYPE_INT && !a.value) {
      result.type  = PPTYPE_INT;
      result.value = (void*)0;
    } else {
      PPResult b = ppeval(*(ASTNode**)at(ast->subnodes, 2), ppenv);
      result.type = PPTYPE_INT;
      if (b.type == PPTYPE_ERROR) {
        result.type  = PPTYPE_ERROR;
        result.value = b.value;
      } else if (a.type == PPTYPE_INT && !a.value) {
        result.value = (void*)0;
      } else {
        result.value = (void*)1;
      }
    }
  } else if (!strcmp(ast->name->content, "operation6")) {
    PPResult a = ppeval(*(ASTNode**)at(ast->subnodes, 0), ppenv);
    if (a.type == PPTYPE_ERROR) {
      result.type  = PPTYPE_ERROR;
      result.value = a.value;
    } else if (a.type == PPTYPE_INT && a.value) {
      result.type  = PPTYPE_INT;
      result.value = (void*)1;
    } else {
      PPResult b = ppeval(*(ASTNode**)at(ast->subnodes, 2), ppenv);
      result.type  = PPTYPE_INT;
      if (b.type == PPTYPE_ERROR) {
        result.type  = PPTYPE_ERROR;
        result.value = b.value;
      } else if (a.type == PPTYPE_INT && a.value) {
        result.value = (void*)1;
      } else {
        result.value = (void*)0;
      }
    }
  } else if (!strcmp(ast->name->content, "operation0")) {
    PPResult  a  = ppeval(*(ASTNode**)at(ast->subnodes, 1), ppenv);
    String   *op = (*(ASTNode**)at(ast->subnodes, 0))->value;

    if (a.type == PPTYPE_ERROR) {
      result.type  = PPTYPE_ERROR;
      result.value = a.value;
    } else if (!strcmp(op->content, "~")) {
      if (a.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(~(long)a.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Bitwise operator '~' only applies to integer values!";
      }
    } else if (!strcmp(op->content, "-") && a.type == PPTYPE_INT) {
      if (a.type == PPTYPE_INT) {
        result.type  = PPTYPE_INT;
        result.value = (void*)(-(long)a.value);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Negation operator '-' only applies to number values!";
      }
    } else if (!strcmp(op->content, "!")) {
      result.type  = PPTYPE_INT;
      result.value = (void*)(long)(!(long)a.value);
    }
  } else if (!strcmp(ast->name->content, "function")) {
    String   *func = (*(ASTNode**)at(ast->subnodes, 0))->value;

    if (!strcmp(func->content, "push")) {
      if (ast->subnodes->size == 4) {
        PPResult a = ppeval(*(ASTNode**)at(ast->subnodes, 2), ppenv);
        result.type  = a.type;
        result.value = a.value;
        push(ppenv->stack, &a);
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Wrong number of parameters for function 'push', expected one!";
      }
    } else if (!strcmp(func->content, "pop")) {
      if (ast->subnodes->size == 4) {
        PPResult *a = pop(ppenv->stack);
        result.type  = a->type;
        result.value = a->value;
      } else {
        result.type  = PPTYPE_ERROR;
        result.value = "Wrong number of parameters for function 'pop', expected none!";
      }
    } else {
      result.type  = PPTYPE_ERROR;
      result.value = "Unknown function!";
    }
  } else if (!strcmp(ast->name->content, "<integer>")) {
    ParsedInteger p = parseinteger(ast->value);
    if (p.valid) {
      result.type  = PPTYPE_INT;
      result.value = (void*)p.integer;
    } else {
      result.type  = PPTYPE_ERROR;
      result.value = "Wrong number format!";
    }
  } else if (!strcmp(ast->name->content, "<decimal>")) {
    /// UNIMPLEMENTED FOR NOW
  } else if (!strcmp(ast->name->content, "<string>")) {
    result.type  = PPTYPE_STRING;
    result.value = newString(ast->value->content);
  } else if (!strcmp(ast->name->content, "<variable>")) {
    result.type  = PPTYPE_INT;
    result.value = NULL;
  }
  return result;
}

String *ppexpandmacro(PPEnv *ppenv, String *expr, Array *trace, int pp) {
  Expansion *e     = newExpansion();
  String    *value = NULL;
  macroexpand(ppenv->env, ppenv->parser, expr, e, NULL, pp);
  switch (e->invalid) {
    case MACRO_ERROR_MAX_DEPTH:
      printmacromessage(ERRLVL_ERROR, trace, e->hist, "Reached maximum expansion depth!");
      break;
    default:
      value = e->value;
      e->value = NULL;
      break;
  }
  deleteExpansion(&e);
  return value;
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
  int           ignore  = 0;
  Array        *ifstack = newArray(sizeof(int));

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
          trim(m.value);
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
        push(ifstack, &def);
        if (!def) {
          ignore = 1;
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
        push(ifstack, &undef);
        if (!undef) {
          ignore = 1;
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
      if (ignore) push(ifstack, &ignore);
      else {
        ASTNode *ast  = newASTNode(NULL, NULL);
        String  *expr = newString("");
        Symbol  *t    = newSymbol(s);
        String  *exp;
        s = ppread(ss, expr);
        if (s) {
          printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
          valid = 0;
        } else {
          exp = ppexpandmacro(ppenv, expr, trace, 1);
          if (exp) {
            StringSymbolStream *sss = sssopen(exp, ppenv->parser);
            while (valid && !(s = sssgets(sss))->eof) {
              astnewsymbol(ast, ppenv->tree, ASTFLAGS_NONE, NULL);
              astnewsymbol(ast, ppenv->tree, ASTFLAGS_NONE, s);
              if (ast->status == STATUS_FAILED) {
                s->line     += t->line;
                s->position += t->position;
                printsymbolmessage(ERRLVL_ERROR, trace, s, "Badely formatted expression!");
                valid = 0;
              }
            }
            astnewsymbol(ast, ppenv->tree, ASTFLAGS_END, NULL);
            if (ast->status != STATUS_CONFIRMED) {
                s->line     += t->line;
                s->position += t->position;
              printsymbolmessage(ERRLVL_ERROR, trace, s, "Badely formatted expression!");
              valid = 0;
            }
            sssclose(sss);
            deleteString(&exp);
          } else valid = 0;
        }
        if (valid) {
          PPResult result = ppeval(ast, ppenv);
          if (result.type == PPTYPE_ERROR) {
            printsymbolmessage(ERRLVL_ERROR, trace, t, result.value);
            valid = 0;
          } else {
            push(ifstack, &result.value);
            if (!result.value) {
              ignore = 1;
            }
          }
        }
        deleteSymbol(&t);
        deleteString(&expr);
        deleteAST(&ast);
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#elif")) {
      if (!*(int*)last(ifstack)) {
        ASTNode *ast  = newASTNode(NULL, NULL);
        String  *expr = newString("");
        Symbol  *t    = newSymbol(s);
        String  *exp;
        s = ppread(ss, expr);
        if (s) {
          printsymbolmessage(ERRLVL_ERROR, trace, s, "Unexpected character '\\'!");
          valid = 0;
        } else {
          exp = ppexpandmacro(ppenv, expr, trace, 1);
          if (exp) {
            StringSymbolStream *sss = sssopen(exp, ppenv->parser);
            while (valid && !(s = sssgets(sss))->eof) {
              astnewsymbol(ast, ppenv->tree, ASTFLAGS_NONE, NULL);
              astnewsymbol(ast, ppenv->tree, ASTFLAGS_NONE, s);
              if (ast->status == STATUS_FAILED) {
                s->line     += t->line;
                s->position += t->position;
                printsymbolmessage(ERRLVL_ERROR, trace, s, "Badely formatted expression!");
                valid = 0;
              }
            }
            astnewsymbol(ast, ppenv->tree, ASTFLAGS_END, NULL);
            if (ast->status != STATUS_CONFIRMED) {
                s->line     += t->line;
                s->position += t->position;
              printsymbolmessage(ERRLVL_ERROR, trace, s, "Badely formatted expression!");
              valid = 0;
            }
            sssclose(sss);
            deleteString(&exp);
          } else valid = 0;
        }
        if (valid) {
          PPResult result = ppeval(ast, ppenv);
          if (result.type == PPTYPE_ERROR) {
            printsymbolmessage(ERRLVL_ERROR, trace, t, result.value);
            valid = 0;
          } else if (result.value) {
            ignore = 0;
            *(int*)last(ifstack) = 1;
          }
        }
        deleteSymbol(&t);
        deleteString(&expr);
        deleteAST(&ast);
      } else { ignore = 1; while ((s = ssgets(ss))->eof && !strcmp(s->text, "\n")); }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#else")) {
      if (!*(int*)last(ifstack)) ignore = 0;
      else                       ignore = 1;
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      if (s->text[0] && s->text[0] != '\n') {
        sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
        printsymbolmessage(ERRLVL_ERROR, trace, s, error);
        valid = 0;
      }
    /////////////////////////////////////////////////////////////////////////////////////
    } else if (!strcmp(s->text, "#endif")) {
      pop(ifstack);
      while ((s = ssgets(ss))->type == SYMBOL_COMMENT);
      if (s->text[0] && s->text[0] != '\n') {
        sprintf(error, "Expected 'newline' got '%s' instead!", s->text);
        printsymbolmessage(ERRLVL_ERROR, trace, s, error);
        valid = 0;
      }
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
          String *exp;
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
          exp = ppexpandmacro(ppenv, str, trace, 0);
          if (exp) {
            for (int i = 0; i < exp->length; i++) fputc(exp->content[i], ppenv->output);
            deleteString(&exp);
          } else valid = 0;
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
  
  deleteArray(&ifstack);
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
  Array        *stack    = newArray(sizeof(PPResult));
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
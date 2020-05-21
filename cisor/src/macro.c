#include <macro.h>

Expansion *newExpansion()
{
  Expansion *e = malloc(sizeof(Expansion));
  if (e) {
    e->value   = newString("");
    e->hist    = newArray(sizeof(Expanded));
    e->invalid = 0;
  }
  return e;
}

void deleteExpansion(Expansion **e)
{
  if (*e) {
    deleteString(&(*e)->value);
    deleteArray(&(*e)->hist);
    free(*e);
    *e = NULL;
  }
}

void freemacro(Macro *m)
{
  deleteString(&m->filename);
  deleteString(&m->name);
  deleteString(&m->value);
  while (m->params->size) {
    deleteString(pop(m->params));
  }
  deleteArray(&m->params);
}

void freeexpansion(Expansion *e)
{
  deleteString(&e->value);
  deleteArray(&e->hist);
}

Symbol *macroconsumestring(StringSymbolStream *sss, String *output)
{
  char c;
  while ((c = tsgetc(sss->tsptr)) != EOF) {
    for (int i = 0; sss->parser->whitespaces[i]; i++) {
      if (c == sss->parser->whitespaces[i]) {
        append(output, c);
        c = 0;
        break;
      }
    }
    if (c) {
      tsungetc(c, sss->tsptr);
      break;
    }
  }
  return sssgets(sss);
}

void macroconcat(String *str, Parser *parser) {
  String *n = newString("");
  String *buffer = NULL;
  char   *tmp;
  int     string = 0;
  int     paste = 0;

  for (int i = 0; i < str->length; i++) {
    char c  = str->content[i];
    int  ws = 0;
    
    if (c == '\n') {
      ws = 1;
      paste = 0;
    } else if (c == '"') {
      string++;
      paste = 0;
    } else if (c == '#') {
      paste++;
    } else {
      for (int j = 0; parser->whitespaces[j]; j++) {
        if (c == parser->whitespaces[j]) {
          ws = 1;
          break;
        }
      }
      if (!ws) {
        if (string == 2) string = 0;
        if (paste  == 2) {
          deleteString(&buffer);
        }
        paste = 0;
      }
    }

    if ((ws && string != 1) || string == 2 || paste > 0) {
      if (!buffer) buffer = newString("");
      append(buffer, c);
    } else if (string == 3) {
      deleteString(&buffer);
      string = 1;
    } else {
      if (buffer) {
        concat(n, buffer);
        buffer = NULL;
      }
      append(n, c);
    }
  }

  if (buffer) concat(n, buffer);
  
  tmp          = n->content;
  n->content   = str->content;
  str->content = tmp;
  str->length  = n->length;
  deleteString(&n);
}

void macroexpand(Array *env, Parser *parser, String *expr, Expansion *e, Array *args, int pp)
{
  StringSymbolStream *sss   = sssopen(expr, parser);
  Symbol             *s;

  if (e->hist->size >= MACRO_EXPANSION_MAX_DEPTH) e->invalid = MACRO_ERROR_MAX_DEPTH;
  while (!e->invalid && (s = macroconsumestring(sss, e->value))->type != SYMBOL_EOF) {
    if (s->type == SYMBOL_VARIABLE) {
      Macro *m;
      int    exp = 0;
      int    arg = 0;
      if (pp && !strcmp(s->text, "defined")) {
        s = sssgets(sss);
        if (strcmp(s->text, "(")) {
          e->invalid = MACRO_ERROR_WRONG_CALL_FORMAT;
        } else {
          s = sssgets(sss);
          for (int i = 0; !arg && i < env->size; i++) {
            m = at(env, i);
            if (!strcmp(s->text, m->name->content)) {
              exp = 1;
              break;
            }
          }
          if (exp) append(e->value, '1');
          else     append(e->value, '0');
          exp = 0;
          s = sssgets(sss);
          if (strcmp(s->text, ")")) {
            e->invalid = MACRO_ERROR_WRONG_PARAMETER_FORMAT;
          }
        }
        arg = 1;
      } else if (args) {
        for (int i = 0; i < args->size; i++) {
          Parameter *p = at(args, i);
          if (s->text[0] == '#') {
            if (!strcmp(s->text + 1, p->name->content)) {
              char *content = p->expansion->value->content;
              int stringize = content[0] != '"' && content[p->expansion->value->length - 1] != '"';
              if (stringize) append(e->value, '"');
              concat(e->value, newString(content));
              if (stringize) append(e->value, '"');
              arg = 1;
            }
          } else if (!strcmp(s->text, p->name->content)) {
              concat(e->value, newString(p->expansion->value->content));
              arg = 1;
          }
        }
      }
      for (int i = 0; !arg && i < env->size; i++) {
        m = at(env, i);
        if (!strcmp(s->text, m->name->content)) {
          Expanded expanded;
          expanded.m        = m;
          expanded.position = e->value->length;
          push(e->hist, &expanded);
          exp = 1;
          break;
        }
      }
      if (exp) {
        if (m->params->size) {
          char c = tsgetc(sss->tsptr);
          if (c == '(') {
            Array *nargs = newArray(sizeof(Parameter));
            do {
              int p = 0;
              String *str = newString("");
              c = tsgetc(sss->tsptr);
              while ((p || (c != ',' && c != ')')) && c != EOF) {
                if (c == '(') p++;
                else if (c == ')') p--;
                append(str, c);
                c = tsgetc(sss->tsptr);
              }
              if (c == EOF) {
                e->invalid = MACRO_ERROR_EOF_IN_PARAMETERS;
              } else {
                trim(str);
                Expansion *sube = newExpansion();
                macroexpand(env, parser, str, sube, args, pp);
                if (!sube->invalid) {
                  if (nargs->size < m->params->size) {
                    Parameter p;
                    p.name      = *(String**)at(m->params, nargs->size);
                    p.expansion = sube;
                    push(nargs, &p);
                  } else {
                    combine(e->hist, sube->hist);
                    deleteExpansion(&sube);
                    e->invalid = MACRO_ERROR_PARAMETER_MISMATCH;
                  }
                } else {
                  combine(e->hist, sube->hist);
                  deleteExpansion(&sube);
                  e->invalid = MACRO_ERROR_WRONG_PARAMETER_FORMAT;
                }
              }
              deleteString(&str);
            } while (c != EOF && c != ')');
            if (!e->invalid) {
              if (nargs->size == m->params->size) {
                String *str = newString(m->value->content);
                macroexpand(env, parser, str, e, nargs, pp);
                deleteString(&str);
              } else {
                e->invalid = MACRO_ERROR_PARAMETER_MISMATCH;
              }
            }
            while (nargs->size) {
              Parameter *p = pop(nargs);
              deleteExpansion(&p->expansion);
            }
            deleteArray(&nargs);
          } else {
            tsungetc(c, sss->tsptr);
            concat(e->value, newString(s->text));
          }
        } else {
          String *str = newString(m->value->content);
          macroexpand(env, parser, str, e, args, pp);
          deleteString(&str);
        }
      } else if (!arg) {
        concat(e->value, newString(s->text));
      }
    } else {
      if (s->open) concat(e->value, newString(s->open));
      concat(e->value, newString(s->text));
      if (s->close) concat(e->value, newString(s->close));
    }
  }
  macroconcat(e->value, parser);
  if (sss) sssclose(sss);
}
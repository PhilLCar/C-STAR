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

String *macroeval(Array *env, Parser *parser, Array *args, Macro *m, Expansion *e)
{

}

void macroexpand(Array *env, Parser *parser, String *expr, Expansion *e)
{
  StringSymbolStream *sss   = sssopen(expr, parser);
  Symbol             *s;

  if (e->hist->size >= MACRO_EXPANSION_MAX_DEPTH) e->invalid = MACRO_ERROR_MAX_DEPTH;
  while (!e->invalid && !(s = macroconsumestring(sss, e->value))->eof) {
    if (s->type == SYMBOL_VARIABLE) {
      Macro *m;
      int    exp = 0;
      for (int i = 0; i < env->size; i++) {
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
            Array *args = newArray(sizeof(String*));
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
                push(args, &str);
              }
            } while (c != EOF && c != ')');
            if (!e->invalid) {
              if (args->size == m->params->size) {
                concat(e->value, macroeval(env, parser, args, m, e));
              } else {
                e->invalid = MACRO_ERROR_PARAMETER_MISMATCH;
              }
            }
            while (args->size) {
              deleteString(pop(args));
            }
            deleteArray(&args);
          } else {
            tsungetc(c, sss->tsptr);
            concat(e->value, newString(s->text));
          }
        } else {
          String *str = newString(m->value->content);
          macroexpand(env, parser, str, e);
          deleteString(&str);
        }
      } else {
        concat(e->value, newString(s->text));
      }
    } else {
      if (s->open) concat(e->value, newString(s->open));
      concat(e->value, newString(s->text));
      if (s->close) concat(e->value, newString(s->close));
    }
  }

  if (sss) sssclose(sss);
}
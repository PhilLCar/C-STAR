#include <bnf_parser.h>

Node *newNode(Node *basenode, char *nodename, type t) {
  Node *new = getnode(basenode, nodename);
  if (new == NULL) {
    new = malloc(sizeof(Node));
    if (new != NULL) {
      new->name = malloc((strlen(nodename) + 1) * sizeof(char));
      if (new->name != NULL) {
	sprintf(new->name, "%s", nodename);
	new->type = t;
	new->num  = 0;
	new->cap  = 0;
      } else {
	free(new);
	new = NULL;
      }
    }
  } else {
    new->type = t;
  }
  return new;
}

void deleteNode(Node **node) {
  if (*node) {
    if ((*node)->content) free((*node)->content);
    free(*node);
  }
  *node = NULL;
}

Node *getnode(Node *basenode, char *nodename) {
  if (basenode == NULL || !strcmp(nodename, "")) return NULL;
  for (int i = 0; i < basenode->num; i++) {
    if (!strcmp(((Node**)basenode->content)[i]->name, nodename)) {
      return ((Node**)basenode->content)[i];
    }
    else if (((Node**)basenode->content)[i]->type != NODE_LEAF) {
      Node *n = getnode(((Node**)basenode->content)[i], nodename);
      if (n != NULL) return n;
    }
  }
  return NULL;
}

void addnode(Node *parent, Node *child) {
  if (parent->type != NODE_ROOT || parent->type != NODE_LIST) {
    // TODO: Have the option to print non-file specific errors and warnings
    //printwarning("bnf_node", "Children was added to parent of wrong type"
  }
  if (parent->cap == 0) {
    parent->cap = 2;
    parent->content = (void*)malloc(parent->cap * sizeof(Node*));
  }
  ((Node**)parent->content)[parent->num] = child;
  parent->num++;
  if (parent->num >= parent->cap) {
    void *t = realloc(parent->content, 2 * parent->cap * sizeof(Node*));
    if (t != NULL) {
      parent->cap *= 2;
      parent->content = t;
    }
  }
}

int parsenode(Node *basenode, Node *node, SymbolStream *ss, char *stop) {
  Symbol *s = getsymbol(ss);
  Node *subnode = newNode(basenode, "", NODE_LIST);
  addnode(node, subnode);
  
  while (strcmp(s->text, stop)) {
    if (!strcmp(s->text, "(")) {
      Node *n = newNode(basenode, "", NODE_ONE_OF);
      addnode(subnode, n);
      parsenode(basenode, n, ss, ")");
    }
    else if (!strcmp(s->text, "{")) {
      Node *n1 = newNode(basenode, "", NODE_MANY_OR_NONE);
      Node *n2 = newNode(basenode, "", NODE_ONE_OF);
      addnode(subnode, n1);
      addnode(n1, n2);
      parsenode(basenode, n2, ss, "}");
    }
    else if (!strcmp(s->text, "[")) {
      Node *n1 = newNode(basenode, "", NODE_ONE_OR_NONE);
      Node *n2 = newNode(basenode, "", NODE_ONE_OF);
      addnode(subnode, n1);
      addnode(n1, n2);
      parsenode(basenode, n2, ss, "]");
    }
    else if (!strcmp(s->text, "'")) {
      Node *n = newNode(basenode, "", NODE_LEAF);
      s = getsymbol(ss);
      n->content = (void*)malloc((strlen(s->text) + 1) * sizeof(char));
      sprintf((char*)n->content, "%s", s->text);
      expect(ss, "'");
      addnode(subnode, n);
    }
    else if (!strcmp(s->text, "\"")) {
      Node *n = newNode(basenode, "", NODE_LEAF);
      s = getsymbol(ss);
      n->content = (void*)malloc((strlen(s->text) + 1) * sizeof(char));
      sprintf((char*)n->content, "%s", s->text);
      expect(ss, "\"");
      addnode(subnode, n);
    }
    else if (!strcmp(s->text, "<")) {
      Node *n;
      s = getsymbol(ss);
      n = newNode(basenode, s->text, NODE_UNKNOWN);
      expect(ss, ">");
      addnode(subnode, n);
    }
    else if (!strcmp(s->text, "|")) {
      subnode = newNode(basenode, "", NODE_LIST);
      addnode(node, subnode);
    }
    else if (!strcmp(s->text, ";")) {
      if (!strcmp(stop, "\n")) {
	char error[256];
	sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"'!", stop);
	printerror(ss->filename, error, s);
	exit(1);
      }
    }
    else if (!strcmp(s->text, "")) {
      return 0;
    }
  }
  return 1;
}

void link(Node *basenode) {
}

Node *parsefile(char *filename) {
  // TODO: Have parser saved to file for quicker recuperation
  Parser       *parser   = newParser("parsing/prs/bnf.prs");
  SymbolStream *ss       = sopen(filename, parser);
  Node         *basenode;
  char          nodename[256];
  
  sprintf(nodename, "root:%s", filename);
  basenode = newNode(NULL, nodename, NODE_ROOT);
  
  while (parseinclude(basenode, ss));
  while (parseline(basenode, ss));

  sclose(ss);
  deleteParser(&parser);

  return basenode;
}

int parseinclude(Node *basenode, SymbolStream *ss) {
  Symbol *s = getsymbol(ss);
  if (!strcmp(s->text, ";;")) {
    expect(ss, "include");
    expect(ss, "(");
    s = getsymbol(ss);
    addnode(basenode, parsefile(s->text));
    expect(ss, ")");
    expect(ss, "\n");
    return 1;
  }
  return 0;
}

int parseline(Node *basenode, SymbolStream *ss) {
  Symbol *s = getsymbol(ss);
  Node *n;
  
  if (!strcmp(s->text, ";")) {
    while (strcmp(s->text, "\n")) {
      s = getsymbol(ss);
      if (strcmp(s->text, "")) return 0;
    }
    return 1;
  }
  if (strcmp(s->text, "<")) {
    printerror(ss->filename, "Expected '"FONT_BOLD"<"FONT_RESET"'!", s);
    exit(1);
  }
  s = getsymbol(ss);
  n = newNode(basenode, s->text, NODE_ONE_OF);
  expect(ss, ">");
  expect(ss, "::=");
  return parsenode(basenode, n, ss, "\n");
}

int expect(SymbolStream *ss, char *str) {
  char error[256];
  Symbol *s;

  if (!strcmp("\n", str)) {
    int junk = 0;
    char prev[256];
    sprintf(prev, "%s", ss->symbol.text);
    s = getsymbol(ss);
    while (strcmp(s->text, str) && strcmp(s->text, "")) {
      s = getsymbol(ss);
      junk = 1;
    }
    if (junk) {
      sprintf(error, "Junk after '"FONT_BOLD"%s"FONT_RESET"'", prev);
      printwarning(ss->filename, error, s);
    }
  }
  else {
    s = getsymbol(ss);
    if (strcmp(s->text, str)) {
      sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"'!", str);
      printerror(ss->filename, error, s);
      exit(1);
    }
    //return 0;
  }

  return 1;
}

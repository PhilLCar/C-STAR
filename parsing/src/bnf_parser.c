#include <bnf_parser.h>

int    include_depth    = 0;
char  *current_filename = NULL;
char **trace            = NULL;

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
	new->content = NULL;
      } else {
	free(new);
	new = NULL;
      }
    }
  } else {
    if (t != NODE_UNKNOWN) {
      new->type = t;
    }
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
    if (((Node**)basenode->content)[i] != NULL) {
      if (!strcmp(((Node**)basenode->content)[i]->name, nodename)) {
	return ((Node**)basenode->content)[i];
      }
      else if (((Node**)basenode->content)[i]->type != NODE_LEAF) {
	Node *n = getnode(((Node**)basenode->content)[i], nodename);
	if (n != NULL) return n;
      }
    }
  }
  return NULL;
}

void addnode(Node *parent, Node *child) {
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
  Symbol *s;
  Node *subnode = newNode(basenode, "", NODE_LIST);
  addnode(node, subnode);
  
  do {
    s = ssgets(ss);
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
      s = ssgets(ss);
      n->content = (void*)malloc((strlen(s->text) + 1) * sizeof(char));
      sprintf((char*)n->content, "%s", s->text);
      expect(ss, "'");
      addnode(subnode, n);
    }
    else if (!strcmp(s->text, "\"")) {
      Node *n = newNode(basenode, "", NODE_LEAF);
      s = ssgets(ss);
      n->content = (void*)malloc((strlen(s->text) + 1) * sizeof(char));
      sprintf((char*)n->content, "%s", s->text);
      expect(ss, "\"");
      addnode(subnode, n);
    }
    else if (!strcmp(s->text, "<")) {
      Node *n;
      s = ssgets(ss);
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
	printsymbolmessage(ERROR, ss->filename, trace, error, s);
	exit(1);
      }
    }
    else if (!strcmp(s->text, "")) {
      return 0;
    }
  } while (strcmp(s->text, stop));
  return 1;
}

void link(Node *basenode) {
  int count = 0;
  for (int i = 0; i < basenode->num; i++) {
    Node *n = ((Node**)basenode->content)[i];
    ((Node**)basenode->content)[i] = NULL;
    if (n != NULL && getnode(basenode, n->name) == NULL) {
      ((Node**)basenode->content)[count++] = n;
    }
  }
  if (count == 0) {
    printnodemessage(WARNING, current_filename, basenode->name, trace, "Node is empty!");
  } else if (count > 1) {
    printnodemessage(WARNING, current_filename, basenode->name, trace, "Multiple roots!");
    for (int i = 0; i < basenode->num; i++) {
      Node *n = ((Node**)basenode->content)[i];
      if (n != NULL) {
	fprintf(stderr, "    :<%s>\n", n->name);
      }
    }
  }
  basenode->num = count;
}

Node *parsefile(char *filename) {
  // TODO: Have parser saved to file for quicker recuperation
  Parser       *parser   = newParser("parsing/prs/bnf.prs");
  SymbolStream *ss       = ssopen(filename, parser);
  Node         *basenode;
  char          nodename[256];

  current_filename = filename;
  sprintf(nodename, "root:%s", filename);
  basenode = newNode(NULL, nodename, NODE_ROOT);
  
  while (parseinclude(basenode, ss));
  while (parseline(basenode, ss));

  ssclose(ss);
  deleteParser(&parser);

  link(basenode);
  return basenode;
}

int parseinclude(Node *basenode, SymbolStream *ss) {
  Symbol *s = ssgets(ss);
  
  // Comment
  if (!strcmp(s->text, ";")) {
    while (strcmp(s->text, "\n")) {
      s = ssgets(ss);
      if (strcmp(s->text, "")) return 0;
    }
    return 1;
  }
  // Emptyline
  if (!strcmp(s->text, "\n")) {
    return 1;
  }
  // Include
  if (!strcmp(s->text, ";;")) {
    expect(ss, "include");
    expect(ss, "(");
    s = ssgets(ss);
    // START INCLUSION
    if (include_depth >= MAX_INCLUDE_DEPTH) {
      printfilemessage(ERROR, current_filename, trace, "Reached maximum inclusion depth!");
      exit(1);
    } else if (include_depth == 0) {
      trace = malloc(MAX_INCLUDE_DEPTH * sizeof(char*));
      memset(trace, 0, MAX_INCLUDE_DEPTH * sizeof(char*));
    }
    trace[include_depth++] = current_filename;
    addnode(basenode, parsefile(s->text));
    trace[include_depth--] = NULL;
    current_filename = trace[include_depth];
    if (include_depth == 0) {
      free(trace);
      trace = NULL;
    } else {
      trace[include_depth] = 0;
    }
    // END INCLUSION
    expect(ss, ")");
    expect(ss, "\n");
    return 1;
  } else ssungets(ss, s);
  return 0;
}

int parseline(Node *basenode, SymbolStream *ss) {
  Symbol *s = ssgets(ss);
  Node *n;

  // Comment
  if (!strcmp(s->text, ";")) {
    while (strcmp(s->text, "\n")) {
      s = ssgets(ss);
      if (strcmp(s->text, "")) return 0;
    }
    return 1;
  }
  // Emptyline
  if (!strcmp(s->text, "\n")) {
    return 1;
  }
  // EOF
  if (!strcmp(s->text, "")) {
    return 0;
  }
  if (strcmp(s->text, "<")) {
    printsymbolmessage(ERROR, ss->filename, trace, "Expected '"FONT_BOLD"<"FONT_RESET"'!", s);
    exit(1);
  }
  s = ssgets(ss);
  n = newNode(basenode, s->text, NODE_ONE_OF);
  expect(ss, ">");
  expect(ss, "::=");
  addnode(basenode, n);
  return parsenode(basenode, n, ss, "\n");
}

int expect(SymbolStream *ss, char *str) {
  char error[256];
  Symbol *s;

  if (!strcmp("\n", str)) {
    int junk = 0;
    char prev[256];
    sprintf(prev, "%s", ss->symbol.text);
    s = ssgets(ss);
    while (strcmp(s->text, str) && strcmp(s->text, "")) {
      s = ssgets(ss);
      junk = 1;
    }
    if (junk) {
      sprintf(error, "Junk after '"FONT_BOLD"%s"FONT_RESET"'", prev);
      printsymbolmessage(ERROR, ss->filename, trace, error, s);
    }
  }
  else {
    s = ssgets(ss);
    if (strcmp(s->text, str)) {
      sprintf(error, "Expected '"FONT_BOLD"%s"FONT_RESET"'!", str);
      printsymbolmessage(ERROR, ss->filename, trace, error, s);
      exit(1);
    }
    //return 0;
  }

  return 1;
}

#include <bnf_parser2.h>
#include <terminal.h>

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

void printnode(BNFNode *n, char *s, int lvl, int last, int exp) {
  char t[256] = "";
  if (lvl) {
    if (last) {
      printf("%s"LAST_NODE, s);
      sprintf(t, "%s ", s);
    } else {
      printf("%s"NODE, s);
      sprintf(t, "%s"BRANCH, s);
    }
  }
  switch (n->type) {
  case NODE_ROOT:
    if (lvl) printf(TEXT_BLUE""FONT_BOLD"[#] %s\n"FONT_RESET, n->name);
    else printf(TEXT_GREEN""FONT_BOLD"[#] %s\n"FONT_RESET, n->name);
    for (int i = 0; i < ((Array*)n->content)->size; i++) {
      printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 1);
    }
    break;
  case NODE_LEAF:
    printf("\"%s\"\n", (char*)n->content);
    break;
  case NODE_LEAF_CONCAT:
    printf("\'%s\'\n", (char*)n->content);
    break;
  case NODE_LIST:
    printf(TEXT_YELLOW"[:]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size; i++) {
      printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 0);
    }
    break;
  case NODE_ONE_OF:
    if (exp || 1) {
      printf(TEXT_MAGENTA""FONT_BOLD"[o] %s\n"FONT_RESET, n->name);
      for (int i = 0; i < ((Array*)n->content)->size; i++) {
	printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 0);
      }
    } else {
      printf(TEXT_MAGENTA"[o] %s\n"FONT_RESET, n->name);
      if (!strcmp(n->name, "")) {
	for (int i = 0; i < ((Array*)n->content)->size; i++) {
	  printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 0);
	}
      }
    }
    break;
  case NODE_ONE_OR_NONE:
    printf(TEXT_GREEN"[+]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size; i++) {
      printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 0);
    }
    break;
  case NODE_MANY_OR_NONE:
    printf(TEXT_RED"[*]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size; i++) {
      printnode(at(n->content, i), t, lvl + 1, i == (((Array*)n->content)->size - 1), 0);
    }
    break;
  }
}

int main() {
  BNFNode *n = parsebnf("parsing/bnf/test.bnf");
  printnode(n, "", 0, 0, 0);
  deleteBNFTree(n, n);
  return 0;
}

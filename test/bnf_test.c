#include <stdio.h>

#include <diagnostic.h>
#include <bnf.h>
#include <terminal.h>

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

void printnode(BNFNode *n, char *s, int base, int lvl, int last) {
  int p = 1;
  if (n->type != NODE_ROOT) {
    if (n->rec >= base) p = 0;
    else (n->rec = base);
  }
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
    base = ++n->rec;
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_LEAF:
    printf("\"%s\"\n", (char*)n->content);
    break;
  case NODE_RAW:
    printf(FONT_BOLD""TEXT_YELLOW"%s\n"FONT_RESET, (char*)n->name);
    break;
  case NODE_LIST:
    printf(TEXT_YELLOW"[:]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_CONCAT:
    printf(TEXT_BLUE"[@]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_ONE_OF:
    printf(TEXT_MAGENTA""FONT_BOLD"[O] %s\n"FONT_RESET, n->name);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_REC:
    printf(TEXT_CYAN"[R] %s\n"FONT_RESET, n->name);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_ONE_OR_NONE:
    printf(TEXT_GREEN"[X]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_MANY_OR_NONE:
    printf(TEXT_RED"[*]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  case NODE_MANY_OR_ONE:
    printf(TEXT_CYAN"[+]\n"FONT_RESET);
    for (int i = 0; i < ((Array*)n->content)->size && p; i++) {
      printnode(*(BNFNode**)at(n->content, i), t, base, lvl + 1, i == (((Array*)n->content)->size - 1));
    }
    break;
  }
}

int main() {
  BNFNode *n = parsebnf("parsing/bnf/test.bnf");
  CHECK_MEMORY;
  printnode(n, "", 0, 0, 0);
  deleteBNFTree(&n);
  CHECK_MEMORY;
  return 0;
}

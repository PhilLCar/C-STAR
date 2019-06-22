#include <bnf_parser.h>
#include <terminal.h>

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

void printnode(Node *n, int lvl) {
  for (int i = 0; i < lvl - 1; i++) {
    printf(BRANCH);
  }
  switch (n->type) {
  case NODE_ROOT:
    printf(TEXT_BLUE""FONT_BOLD"[#] %s\n"FONT_RESET, n->name);
    for (int i = 0; i < n->num; i++) {
      printnode(((Node**)n->content)[i], lvl + 1);
      printf("what?\n");
    }
    break;
  case NODE_LEAF:
    printf("%s\n", (char*)n->content);
    break;
  case NODE_LIST:
    printf("::\n");
    for (int i = 0; i < n->num; i++) {
      printnode(((Node**)n->content)[i], lvl + 1);
    }
    break;
  case NODE_ONE_OF:
    printf(TEXT_BLUE"%s\n"FONT_RESET, n->name);
    for (int i = 0; i < n->num; i++) {
      printnode(((Node**)n->content)[i], lvl + 1);
    }
    break;
  case NODE_ONE_OR_NONE:
    printf("[+]");
    for (int i = 0; i < n->num; i++) {
      printnode(((Node**)n->content)[i], lvl + 1);
    }
    break;
  case NODE_MANY_OR_NONE:
    printf("[*]");
    for (int i = 0; i < n->num; i++) {
      printnode(((Node**)n->content)[i], lvl + 1);
    }
    break;
  case NODE_UNKNOWN:
    printf("UNKNOWN\n");
    break;
  }
}

int main() {
  Node *n = parsefile("parsing/bnf/test.bnf");
  printnode(n, 0);
  return 0;
}

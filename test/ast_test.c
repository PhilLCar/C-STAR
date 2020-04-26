#include <ast.h>

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

void printnode(ASTNode *node, char *tab, int lvl, int last) {
  char t[256] = "";
  if (lvl) {
    if (last) {
      printf("%s"LAST_NODE, tab);
      sprintf(t, "%s ", tab);
    } else {
      printf("%s"NODE, tab);
      sprintf(t, "%s"BRANCH, tab);
    }
  }
  if (node->name->content[0]) {
    printf(TEXT_BLUE""FONT_BOLD"[ %s ]\n"FONT_RESET, node->name->content);
  } else if (node->value->content[0]) {
    printf(TEXT_GREEN""FONT_BOLD"\"%s\"\n"FONT_RESET, node->value->content);
  } else {
    printf("+\n");
  }
  for (int i = 0; i < node->subnodes->size; i++) {
    printnode(at(node->subnodes, i), t, lvl + 1, i == node->subnodes->size - 1);
  }
}

int main()
{
    ASTNode *n = parseast("csr/test.csr");
    printnode(n, "", 0, 0);
    //deleteASTTree(&n);
    return 0;
}
#include <stdio.h>

#include <diagnostic.h>
#include <ast.h>

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

char *printstatus(ASTNode *ast) {
  if (ast->ref) {
    switch(ast->status) {
      case STATUS_CONFIRMED:
        return TEXT_GREEN"[C]"FONT_RESET;
      case STATUS_FAILED:
        return TEXT_RED"[F]"FONT_RESET;
      case STATUS_NOSTATUS:
        return TEXT_WHITE"[N]"FONT_RESET;
      case STATUS_ONGOING:
        return TEXT_MAGENTA"[O]"FONT_RESET;
      case STATUS_PARTIAL:
        return TEXT_YELLOW"[P]"FONT_RESET;
      case STATUS_REC:
        return TEXT_CYAN"[R]"FONT_RESET;
      case STATUS_SKIP:
        return TEXT_BLUE"[S]"FONT_RESET;
    }
  } else {
    switch(ast->status) {
      case STATUS_CONFIRMED:
        return FONT_BOLD""TEXT_GREEN"[C]"FONT_RESET;
      case STATUS_FAILED:
        return FONT_BOLD""TEXT_RED"[F]"FONT_RESET;
      case STATUS_NOSTATUS:
        return FONT_BOLD""TEXT_WHITE"[N]"FONT_RESET;
      case STATUS_ONGOING:
        return FONT_BOLD""TEXT_MAGENTA"[O]"FONT_RESET;
      case STATUS_PARTIAL:
        return FONT_BOLD""TEXT_YELLOW"[P]"FONT_RESET;
      case STATUS_REC:
        return FONT_BOLD""TEXT_CYAN"[R]"FONT_RESET;
      case STATUS_SKIP:
        return FONT_BOLD""TEXT_BLUE"[S]"FONT_RESET;
    }
  }
  return "[U]";
}

char *printmod(ASTNode *n) {
  return "";
}

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
  if (node->name->content[0] && node->value->content[0]) {
    printf("%s%s "TEXT_BLUE""FONT_BOLD"(%s: "FONT_RESET, printstatus(node), printmod(node), node->name->content);
    printf(TEXT_GREEN""FONT_BOLD"\"%s\""TEXT_BLUE")\n"FONT_RESET, node->value->content);
  } else if (node->name->content[0]) {
    printf("%s%s "TEXT_BLUE""FONT_BOLD"(%s)\n"FONT_RESET, printstatus(node), printmod(node), node->name->content);
  } else if (node->value->content[0]) {
    printf("%s%s "TEXT_GREEN""FONT_BOLD"\"%s\"\n"FONT_RESET, printstatus(node), printmod(node), node->value->content);
  } else {
    printf("%s%s\n", printstatus(node), printmod(node));
  }
  for (int i = 0; i < node->subnodes->size; i++) {
    printnode(*(ASTNode**)at(node->subnodes, i), t, lvl + 1, i == node->subnodes->size - 1);
  }
}

int main()
{
  CHECK_MEMORY;
  ASTNode *n = parseast("unit-tests/test.csr");
  printnode(n, "", 0, 0);
  deleteAST(&n);
  CHECK_MEMORY;
  return 0;
}
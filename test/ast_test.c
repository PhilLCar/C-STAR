#include <stdio.h>

#include <diagnostic.h>
#include <ast.h>
#include <terminal.h>

#ifdef WIN
#include <windows.h>
#endif

#define BRANCH    "│"
#define NODE      "├"
#define LAST_NODE "└"

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
    printf(TEXT_GREEN"[O]"FONT_RESET"%s "TEXT_BLUE""FONT_BOLD"(%s: "FONT_RESET, printmod(node), node->name->content);
    printf(TEXT_GREEN""FONT_BOLD"\"%s\""TEXT_BLUE")\n"FONT_RESET, node->value->content);
  } else if (node->name->content[0]) {
    printf(TEXT_GREEN"[O]"FONT_RESET"%s "TEXT_BLUE""FONT_BOLD"(%s)\n"FONT_RESET, printmod(node), node->name->content);
  } else if (node->value->content[0]) {
    printf(TEXT_GREEN"[O]"FONT_RESET"%s "TEXT_GREEN""FONT_BOLD"\"%s\"\n"FONT_RESET, printmod(node), node->value->content);
  } else {
    printf(TEXT_GREEN"[O]"FONT_RESET"%s\n", printmod(node));
  }
  for (int i = 0; i < node->subnodes->size; i++) {
    printnode(*(ASTNode**)at(node->subnodes, i), t, lvl + 1, i == node->subnodes->size - 1);
  }
}

int main()
{
  #ifdef WIN
  SetConsoleOutputCP(CP_UTF8);
  #endif
  CHECK_MEMORY;
  ASTNode *n = parseast("unit-tests/test.csr");
  printnode(n, "", 0, 0);
  deleteAST(&n);
  CHECK_MEMORY;
  STOP_WATCHING;
  return 0;
}
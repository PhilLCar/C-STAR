#include <stdio.h>

#include <dir.h>
#include <terminal.h>

void setupUT(Coordinate *coord) {
  *coord = getTerminalSize();
  printf("%d:%d\n", coord->x, coord->y);
}

int main(void) {
  Array      *tests = directory("unit-tests/*.c*");
  Coordinate  coord;

  setupUT(&coord);

  for (int i = 0; i < tests->size; i++) {
    DirectoryItem *di = at(tests, i);
    if (di->type == DIRITEM_FILE) {
      
    }
  }

  while (tests->size) popobj(tests, (F)freedi);
  deleteArray(&tests);

  CHECK_MEMORY;
  STOP_WATCHING;
  return 0;
}
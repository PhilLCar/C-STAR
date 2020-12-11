#include <terminal.h>

#include <stdio.h>

#ifdef WIN
#include <windows.h>
#error "getTerminalSize(): UNIMPLEMENTED"
Coordinate getTerminalSize() {
  
}
#else
#include <sys/ioctl.h>
#include <unistd.h>

Coordinate getTerminalSize() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return (Coordinate){ w.ws_col, w.ws_row };
}
#endif

void placeCursor(int x, int y) {
  printf("\x1b[%d;%dH", x, y);
}

void pushCursor() {
  printf("\x1b[s");
}

void popCursor() {
  printf("\x1b[u");
}
#include <terminal.h>

#include <stdio.h>

#ifdef WIN
#include <windows.h>

Coordinate getTerminalSize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
    rows    = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;

    return (Coordinate){ columns, rows };
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

void clearTerminal() {
  printf("\x1b[2J");
  fflush(stdout);
}

void placeCursor(int x, int y) {
  printf("\x1b[%d;%dH", x, y);
  fflush(stdout);
}

void moveCursor(int x, int y) {
  if (x > 0)      printf("\x1b[%dC",  x);
  else if (x < 0) printf("\x1b[%dD", -x);
  if (y > 0)      printf("\x1b[%dB",  y);
  else if (y < 0) printf("\x1b[%dA", -y);
  fflush(stdout);
}

void pushCursor() {
  printf("\x1b[s");
  fflush(stdout);
}

void popCursor() {
  printf("\x1b[u");
  fflush(stdout);
}
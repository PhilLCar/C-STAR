#include <stdio.h>
#include <string.h>

#include <dir.h>
#include <terminal.h>

typedef enum outcome {
  OUTCOME_PASS,
  OUTCOME_COMPILE_ERROR,
  OUTCOME_EXECUTE_ERROR,
  OUTCOME_UNKNOWN_ERROR,
} Outcome;

typedef struct results {
  int total;
  int pass;
  int cerr;
  int eerr;
  int uerr;
} Results;

void getVersion(char *version) {
  FILE *v   = popen("bin/cisor -v", "r");
  char *tag = "Version: ";
  int   found;

  if (v) {
    char c;
    do {
      found = 1;
      for (int i = 0; tag[i]; i++) {
        c = fgetc(v);
        if (c != tag[i]) {
          found = 0;
          break;
        }
      }
      if (found) {
        for (int i = 0; (c = fgetc(v)) != EOF && c != ' '; i++) {
          version[i] = c;
        }
      }
    } while (!found && c != EOF);

    fclose(v);
  }
}

void printReport(Coordinate *coord, char *version, Results *results) {
  printf(CORNER_TL_DOUBLE);
  for (int i = 0; i < coord->x - 2; i++) {
    printf(LINE_H_DOUBLE);
  }
  printf(CORNER_TR_DOUBLE"\n");
  for (int j = 0; j < 4; j++) {
    if (j == 1) {
      printf(TEE_L_DOUBLE_TB);
      for (int i = 0; i < coord->x - 2; i++) {
        printf(LINE_H);
      }
      printf(TEE_R_DOUBLE_TB"\n");
    } else {
      printf(LINE_V_DOUBLE);
      for (int i = 0; i < coord->x - 2; i++) {
        printf(" ");
      }
      printf(LINE_V_DOUBLE"\n");
    }
  }
  printf(CORNER_BL_DOUBLE);
  for (int i = 0; i < coord->x - 2; i++) {
    printf(LINE_H_DOUBLE);
  }
  printf(CORNER_BR_DOUBLE"\n");
  pushCursor();
  moveCursor(2, -5);
  printf(FONT_BOLD"CISOR"FONT_RESET" (version: %s) "FONT_BOLD""TEXT_YELLOW"UNIT-TESTS"FONT_RESET"\n", version);
  moveCursor(2, 1);
  printf(FONT_BOLD""TEXT_GREEN"Passed: "FONT_RESET" %3d / %3d (%.2f%%)\n", results->pass, results->total, (double)results->pass/(double)results->total);
  moveCursor(2, 0);
  printf(FONT_BOLD""TEXT_BLUE"Unknown:"FONT_RESET" %3d / %3d (%.2f%%)\n", results->uerr, results->total, (double)results->uerr/(double)results->total);
  moveCursor(coord->x / 2, -2);
  printf(FONT_BOLD""TEXT_RED"Compilation error:"FONT_RESET" %3d / %3d (%.2f%%)\n", results->cerr, results->total, (double)results->cerr/(double)results->total);
  moveCursor(coord->x / 2, 0);
  printf(FONT_BOLD""TEXT_MAGENTA"Execution error:  "FONT_RESET" %3d / %3d (%.2f%%)\n", results->cerr, results->total, (double)results->cerr/(double)results->total);
  popCursor();
}

void executeUT(Coordinate *coord, char *version) {
  Coordinate tmp = getTerminalSize();
  if (tmp.x != coord->x || tmp.y != coord->y) {
    clearTerminal();
    *coord = tmp;
  }
}

int main(void) {
  Array      *tests      = directory("unit-tests/*.c*");
  Coordinate  coord      = getTerminalSize();
  Results     results    = { tests->size, 0, 0, 0, 0 };
  char        version[8] = { '\0' };

  getVersion(version);
  printReport(&coord, version, &results);

  for (int i = 0; i < tests->size; i++) {
    DirectoryItem *di = at(tests, i);
    if (di->type == DIRITEM_FILE) {
      //executeUT(&coord, version);
    }
  }

  while (tests->size) popobj(tests, (F)freedi);
  deleteArray(&tests);

  CHECK_MEMORY;
  STOP_WATCHING;
  return 0;
}
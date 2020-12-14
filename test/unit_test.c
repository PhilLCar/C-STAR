#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dir.h>
#include <file.h>
#include <terminal.h>

#ifdef WIN
#include <windows.h>
#define  popen _popen
#endif

typedef enum status {
  STATUS_COMPILING,
  STATUS_COMPILE_FAILED,
  STATUS_EXECUTING,
  STATUS_EXECUTE_FAILED,
  STATUS_UNKNOWN,
  STATUS_PASSED
} Status;

typedef struct results {
  int total;
  int pass;
  int cerr;
  int eerr;
  int uerr;
} Results;

void getVersion(char *version) {
  #ifdef WIN
  FILE *v   = popen("bin\\CISOR.exe -v", "r");
  #else
  FILE *v   = popen("bin/cisor -v", "r");
  #endif
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
  printf(FONT_BOLD""TEXT_GREEN"Passed: "FONT_RESET" %3d / %3d (%.2f%%)\n", results->pass, results->total, 100.0 * (double)results->pass / (double)results->total);
  moveCursor(2, 0);
  printf(FONT_BOLD""TEXT_BLUE"Unknown:"FONT_RESET" %3d / %3d (%.2f%%)\n", results->uerr, results->total, 100.0 * (double)results->uerr / (double)results->total);
  moveCursor(coord->x / 2, -2);
  printf(FONT_BOLD""TEXT_RED"Compilation error:"FONT_RESET" %3d / %3d (%.2f%%)\n", results->cerr, results->total, 100.0 * (double)results->cerr / (double)results->total);
  moveCursor(coord->x / 2, 0);
  printf(FONT_BOLD""TEXT_MAGENTA"Execution error:  "FONT_RESET" %3d / %3d (%.2f%%)\n", results->eerr, results->total, 100.0 * (double)results->eerr / (double)results->total);
  popCursor();
  {
    double completion = (double)(results->pass + results->uerr + results->cerr + results->eerr) / (double)results->total;
    char   message[64];
    int    mstart, mend, bend;
    if (completion == 1.0) sprintf(message, "COMPLETED");
    else                   sprintf(message, "PROGRESS %.2f%%", 100.0 * completion);
    mend   = (int)strlen(message);
    mstart = (coord->x - 2 - mend) / 2;
    mend   += mstart;
    bend   = (int)((double)(coord->x - 2) * completion);
    printf("["BG_GREEN);
    for (int i = 0; i < coord->x - 2; i++) {
      if (i < mstart || i >= mend) printf(" ");
      else                         printf("%c", message[i - mstart]);
      if (i == bend)               printf(FONT_RESET);
    }
    printf(FONT_RESET"]\n");
  }
}

void printStatus(Coordinate *coord, char *version, Results *results, char *test, Status status) {
  char *status_name;
  switch (status) {
    case STATUS_COMPILING:
      status_name = TEXT_CYAN" COMPILING "FONT_RESET;
      break;
    case STATUS_COMPILE_FAILED:
      status_name = TEXT_RED" COMP FAIL "FONT_RESET;
      break;
    case STATUS_EXECUTING:
      status_name = TEXT_YELLOW" EXECUTING "FONT_RESET;
      break;
    case STATUS_EXECUTE_FAILED:
      status_name = TEXT_MAGENTA" EXEC_FAIL "FONT_RESET;
      break;
    case STATUS_PASSED:
      status_name = TEXT_GREEN"  SUCCESS  "FONT_RESET;
      break;
    default:
    case STATUS_UNKNOWN:
      status_name = TEXT_BLUE"  UNKNOWN  "FONT_RESET;
      break;
  }
  moveCursor(0, -8);
  if (status == STATUS_COMPILING || status == STATUS_EXECUTING) printf(FONT_BOLD"%s "FONT_RESET, test);
  else                                                          printf("%s ",                    test);
  for (int i = 0; i < coord->x - strlen(test) - 15; i++) {
    printf(".");
  }
  printf(" [%s]\n", status_name);
  printReport(coord, version, results);
}

void executeUT(Coordinate *coord, char *version, Results *results, char *test) {
  Coordinate tmp    = getTerminalSize();
  Status     status = STATUS_COMPILING;
  if (tmp.x != coord->x || tmp.y != coord->y) {
    clearTerminal();
    *coord = tmp;
  }
  printf("\n");
  printStatus(coord, version, results, test, status);
  {
    char *filename = filenamewoext(test);
    char  output[1024]; 
    char  command[1024];
    #ifdef WIN
    sprintf(output,  "out\\%s.exe", filenamewopath(filename));
    sprintf(command, "bin\\CISOR.exe %s -o %s >nul 2>&1", test, output);
    #else
    sprintf(output,  "out/%s.out", filenamewopath(filename));
    sprintf(command, "bin/cisor %s -o %s >/dev/null 2>&1", test, output);
    #endif
    pushCursor();
    system(command);
    popCursor();
    printStatus(coord, version, results, test, STATUS_EXECUTING);
    if (fileexists(output, FILE_EXECUTE)) {
      FILE *expected = fopen(test,   "r");
      FILE *actual   = popen(output, "r");
      char *tag      = "/* Expected output:\n";

      if (expected && actual) {
        for (int i = 0; tag[i]; i++) {
          if (fgetc(expected) != tag[i]) {
            results->uerr++;
            status = STATUS_UNKNOWN;
            break;
          }
        }
        if (status != STATUS_UNKNOWN) {
          char c;
          while ((c = fgetc(actual)) == fgetc(expected) && c != EOF);
          if (c == EOF) {
            results->pass++;
            status = STATUS_PASSED;
          } else {
            results->eerr++;
            status = STATUS_EXECUTE_FAILED;
          }
        }
      } else {
        results->uerr++;
        status = STATUS_UNKNOWN;
      }

      if (expected) fclose(expected);
      if (actual)   fclose(actual);
    } else {
      results->cerr++;
      status = STATUS_COMPILE_FAILED;
    }
    free(filename);
  }
  printStatus(coord, version, results, test, status);
}

int main(void) {
  Array      *tests      = directory("unit-tests/*.c*");
  Coordinate  coord      = getTerminalSize();
  Results     results    = { tests->size, 0, 0, 0, 0 };
  char        version[8] = { '\0' };

  #ifdef WIN
  SetConsoleOutputCP(CP_UTF8);
  #endif

  getVersion(version);
  printReport(&coord, version, &results);
  
  for (int i = 0; i < tests->size; i++) {
    DirectoryItem *di = at(tests, i);
    if (di->type == DIRITEM_FILE) {
      executeUT(&coord, version, &results, di->name);
    }
  }

  while (tests->size) popobj(tests, (F)freedi);
  deleteArray(&tests);

  CHECK_MEMORY;
  STOP_WATCHING;
  return 0;
}
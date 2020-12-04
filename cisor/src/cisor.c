#include <cisor.h>

#include <string.h>

#include <preprocessor.h>
#include <intermediate.h>
#include <file.h>

Options *parseargs(int argc, char *argv[]) {
  Options *options = malloc(sizeof(Options));
  memset(options, 0, sizeof(Options));

  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case '-':
        if (!strcmp(argv[i], "--version")) {
          options->version = 1;
        } else if (!strcmp(argv[i], "--help")) {
          options->help = 1;
        } else if (!strcmp(argv[i], "--debug")) {
          options->debug = 1;
        }
        break;
      case 'v':
        options->version = 1;
        break;
      case 'h':
        options->help = 1;
        break;
      case 'g':
        options->debug = 1;
        break;
      case 'o':

        break;
      case 'E':
      case 'e':
        options->preprocessed = 1;
        break;
      case 'S':
      case 's':
        options->assembly = 1;
        break;
      case 'C':
      case 'c':
        options->compiled = 1;
        break;
      }
    }
  }
  return options;
}

void printversion() {
  #ifdef WIN
  printf("cisor (Windows x64) - C* compiler\n");
  #else
  printf("cisor (Linux x64) - C* compiler\n");
  #endif
  printf("Version: 0.0.0 (2020-12-04)\n");
  printf("Author: Philippe Caron\n");
}

void printhelp(char *name) {
  printf("Usage: %s [options] files...\n", name);
  printf("Options:\n");
  printf("  -v, --version        Display the compiler version information.\n");
  printf("  -h, --help           Display this information.\n");
  printf("  -g, --debug          Debug symbols.\n");
  printf("  -o                   Specify output file.\n");
  printf("  -I<path>             Add path to include directory.\n");
  printf("  -D<variable>         Defined a variable for the preprocessor.\n");
  printf("  -E, -e               Preprocess only.\n");
  printf("  -S, -s               Preprocess and compile only.\n");
  printf("  -C, -c               Preprocess, compile and assemble only.\n");
}

int main(int argc, char *argv[]) {
  Options *options = parseargs(argc, argv);

  if (options->version) {
    printversion();
  } else if (options->help) {
    printhelp(argv[0]);
  }

  free(options);
  return 0;
}
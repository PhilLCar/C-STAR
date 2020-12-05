#include <cisor.h>

#include <string.h>

#include <preprocessor.h>
#include <intermediate.h>
#include <compiler.h>
#include <assembler.h>
#include <linker.h>

Options *parseargs(int argc, char *argv[]) {
  Options *options = malloc(sizeof(Options));
  memset(options, 0, sizeof(Options));

  options->includepath = newArray(sizeof(char*));
  options->inputs      = newArray(sizeof(char*));
  options->definitions = newArray(sizeof(char*));

  options->preprocess = 1;
  options->compile    = 1;
  options->assemble   = 1;
  options->link       = 1;

  for (int i = 1; i < argc; i++) {
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
        if (++i >= argc) {
          fprintf(stderr, "Expected filename after '-o'!\n");
        } else {
          options->output = argv[i];
        }
        break;
      case 'I':
        {
          char *inc = argv[i] + 2;
          push(options->includepath, &inc);
        }
        break;
      case 'D':
        {
          char *def = argv[i] + 2;
          push(options->definitions, &def);
        }
        break;
      case 'E':
      case 'e':
        options->preprocess = 1;
        options->compile    = 0;
        options->assemble   = 0;
        options->link       = 0;
        break;
      case 'S':
      case 's':
        options->preprocess = 1;
        options->compile    = 1;
        options->assemble   = 0;
        options->link       = 0;
        break;
      case 'C':
      case 'c':
        options->preprocess = 1;
        options->compile    = 1;
        options->assemble   = 1;
        options->link       = 0;
        break;
      default:
        fprintf(stderr, "Unknown option: '%s'!", argv[i]);
        break;
      }
    }
    push(options->inputs, & argv[i]);
  }
  if (!options->output) options->output = "a.out";
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
  } else {
    if (options->preprocess) {
      preprocess(options);
    }
    if (options->compile) {
      intermediate(options);
      compile(options);
    }
    if (options->assemble) {
      assemble(options);
    }
    if (options->link) {
      link(options);
    }
  }

  deleteArray(&options->inputs);
  deleteArray(&options->includepath);
  deleteArray(&options->definitions);
  free(options);
  return 0;
}
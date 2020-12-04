#include <preprocessor.h>
#include <diagnostic.h>

int main() {
  char    *input    = "unit-tests/include-test.csr";
  Array   *includes = newArray(sizeof(char*));
  Array   *inputs   = newArray(sizeof(char*));
  Options  options;

  push(inputs, &input);
  
  options.inputs      = inputs;
  options.output      = "unit-tests/include-test";
  options.includepath = includes;
  
  CHECK_MEMORY;

  preprocess(&options);

  deleteArray(&includes);
  deleteArray(&inputs);

  CHECK_MEMORY;
  STOP_WATCHING;
}
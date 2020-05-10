#include <preprocessor.h>
#include <diagnostic.h>

int main() {
  Array *includes = newArray(sizeof(char*));
  
  CHECK_MEMORY;

  preprocess("unit-tests/include-test.csr", includes);
  deleteArray(&includes);

  CHECK_MEMORY;
}
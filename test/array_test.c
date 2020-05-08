#include <stdio.h>

#include <diagnostic.h>
#include <array.h>

int main() {
  Array *chars = newArray(sizeof(char));
  Array *ints  = newArray(sizeof(int));
  char a = 'a', b = 'b', c = 'c';
  int  i1 = 12, i2 = 54, i3 = 92;
  push(chars, &a);
  push(chars, &b);
  push(chars, &c);
  push(ints, &i1);
  push(ints, &i2);
  push(ints, &i3);

  printf("Chars[2]:  %c, element: %ld, size: %d, capacity: %d\n",
	       ((char*)chars->content)[1],
	       chars->element_size,
	       chars->size,
	       chars->capacity);
  for (int i = 0; i < chars->size; i++) {
    printf("\t%d -> %c\n", i, *(char*)at(chars, i));
  }

  int i4 = *(int*)rem(ints, 1);
  insert(ints, 0, &i4);
  printf("Ints [3]: %d, element: %ld, size: %d, capacity: %d\n",
	       i4,
	       ints->element_size,
	       ints->size,
	       ints->capacity);
  for (int i = 0; i < ints->size; i++) {
    printf("\t%d -> %d\n", i, *(int*)at(ints, i));
  }

  printf("92?: %d\n", in(ints, &i4));
  i4 = 33;
  printf("33?: %d\n", in(ints, &i4));
  
  CHECK_MEMORY;

  deleteArray(&chars);
  deleteArray(&ints);
  
  CHECK_MEMORY;

  return 0;
}

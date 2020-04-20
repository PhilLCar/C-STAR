#include <stdio.h>

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
  int i4 = *(int*)pop(ints);
  printf("Chars[2]:  %c, element: %ld, size: %d, capacity: %d\n",
	 ((char*)chars->content)[1],
	 chars->element_size,
	 chars->size,
	 chars->capacity);
  printf("Ints [3]: %d, element: %ld, size: %d, capacity: %d\n",
	 i4,
	 ints->element_size,
	 ints->size,
	 ints->capacity);
  deleteArray(&chars);
  deleteArray(&ints);
  return 0;
}

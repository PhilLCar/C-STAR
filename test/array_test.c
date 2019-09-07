#include <stdio.h>

#include <array.h>

int main() {
  Array *chars = newArray(sizeof(char), 2);
  Array *ints  = newArray(sizeof(int),  8);
  char a = 'a', b = 'b', c = 'c';
  int  i1 = 12, i2 = 54, i3 = 92;
  append(chars, &a);
  append(chars, &b);
  append(chars, &c);
  append(ints, &i1);
  append(ints, &i2);
  append(ints, &i3);
  int *i4 = (int*)rmlast(ints);
  printf("Chars[2]:  %c, element: %d, size: %d, capacity: %d\n",
	 ((char*)chars->content)[1],
	 chars->element_size,
	 chars->size,
	 chars->capacity);
  printf("Ints [3]: %d, element: %d, size: %d, capacity: %d\n",
	 *i4,
	 ints->element_size,
	 ints->size,
	 ints->capacity);
  deleteArray(&chars);
  deleteArray(&ints);
  return 0;
}

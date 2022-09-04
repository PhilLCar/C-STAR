#ifndef OOP_UTILS
#define OOP_UTILS

#define buildable(TYPE, type, ...) \
int cons ## type(struct type *ptr, __VA_ARGS__);\
void free ## type(struct type *ptr);\
TYPE *new ## TYPE(__VA_ARGS__); \
void delete ## TYPE(TYPE **ptr_location)

#endif
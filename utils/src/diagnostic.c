#ifdef MEMORY_WATCH

#define UTILS_DIAGNOSTIC_INC

#include <diagnostic.h>

size_t _mem_total_size = 0;
int    _mem_table_size = 0;
int    _mem_table_cap  = 1024;
void **_mem_table      = NULL;

void *__malloc(size_t size) {
  if (_mem_table == NULL) {
    _mem_table = (void**)malloc(_mem_table_cap * 2 * sizeof(void*));
  } else if (_mem_table_size >= _mem_table_cap) {
    _mem_table_cap *= 2;
    _mem_table = (void**)realloc(_mem_table, _mem_table_cap * 2 * sizeof(void*));
  }
  void *mem = malloc(size);
  if (mem) {
    _mem_total_size += size;
    _mem_table[2 * _mem_table_size    ] = mem;
    _mem_table[2 * _mem_table_size + 1] = (void*)size;
    _mem_table_size++;
  }
  return mem;
}

void __free(void *mem) {
  size_t size;
  for (int i = 0; i < 2 * _mem_table_size; i += 2) {
    if (_mem_table[i] == mem) {
      size = (size_t)_mem_table[i + 1];
      _mem_table[i    ] = NULL;
      _mem_table[i + 1] = NULL;
      break;
    }
  }
  _mem_total_size -= size;
  free(mem);
}

void *__realloc(void *mem, size_t size) {
  for (int i = 0; i < 2 * _mem_table_size; i += 2) {
    if (_mem_table[i] == mem) {
      _mem_total_size += size - (size_t)_mem_table[i + 1];
      _mem_table[i    ] = realloc(mem, size);
      _mem_table[i + 1] = (void*)size;
      return _mem_table[i];
    }
  }
  return NULL;
}

size_t memuse() {
    return _mem_total_size;
}

#endif
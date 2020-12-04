#ifdef MEMORY_WATCH
#define UTILS_DIAGNOSTIC_INC
#include <diagnostic.h>

size_t _mem_total_size = 0;
int    _mem_table_size = 0;
int    _mem_table_cap  = 2048;
void **_mem_table      = NULL;

void *__malloc(size_t size, char *file, int line) {
  if (_mem_table == NULL) {
    _mem_table = (void**)malloc(_mem_table_cap * sizeof(void*));
  }
  void *mem = malloc(size);
  if (mem) {
    _mem_total_size += size;
    for (int i = 0; i < _mem_table_size; i += 4) {
      if (!_mem_table[i]) {
        _mem_table[i]     = mem;
        _mem_table[i + 1] = (void*)size;
        _mem_table[i + 2] = file;
        _mem_table[i + 3] = (void*)(long)line;
        return mem;
      }
    }
    if (_mem_table_size >= _mem_table_cap) {
      _mem_table_cap <<= 1;
      _mem_table = (void**)realloc(_mem_table, _mem_table_cap * sizeof(void*));
    }
    _mem_table[_mem_table_size]     = mem;
    _mem_table[_mem_table_size + 1] = (void*)size;
    _mem_table[_mem_table_size + 2] = file;
    _mem_table[_mem_table_size + 3] = (void*)(long)line;
    _mem_table_size += 4;
  }
  return mem;
}

void __free(void *mem) {
  size_t size;
  for (int i = 0; i < _mem_table_size; i += 4) {
    if (_mem_table[i] == mem) {
      size = (size_t)_mem_table[i + 1];
      _mem_table[i]     = NULL;
      _mem_table[i + 1] = NULL;
      _mem_table[i + 2] = NULL;
      _mem_table[i + 3] = NULL;
      break;
    }
  }
  _mem_total_size -= size;
  free(mem);
}

void *__realloc(void *mem, size_t size, char *file, int line) {
  for (int i = 0; i < _mem_table_size; i += 4) {
    if (_mem_table[i] == mem) {
      _mem_total_size += size - (size_t)_mem_table[i + 1];
      _mem_table[i]     = realloc(mem, size);
      _mem_table[i + 1] = (void*)size;
      _mem_table[i + 2] = file;
      _mem_table[i + 3] = (void*)(long)line;
      return _mem_table[i];
    }
  }
  return NULL;
}

size_t __memuse() {
    return _mem_total_size;
}

void __end() {
  for (int i = 0; i < _mem_table_size; i += 4) {
    if (_mem_table[i]) {
      fprintf(stderr, "%s(%ld): A block of size %ld was allocated but never recovered!\n",
                      (char*)_mem_table[i + 2],
                      (long) _mem_table[i + 3],
                      (long) _mem_table[i + 1]);
    }
  }
  free(_mem_table);
}

#endif
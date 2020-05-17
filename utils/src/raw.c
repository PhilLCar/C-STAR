#include <raw.h>

long intpow(int base, int power)
{
  long acc = 1;
  for (int i = 0; i < power; i++) {
    acc *= base;
  }
  return acc;
}

ParsedInteger parseinteger(String *str)
{
  ParsedInteger p;
  int len   = str->length - 1;
  p.integer = 0;

  if (len >= 0) {
    switch (str->content[len]) {
      case 'b':
      case 'B':
        if (--len >= 0) {
          if (str->content[len] == 's' || str->content[len] == 'S') {
            p.valid = RAW_BYTE;
          } else if (str->content[len] == 'u' || str->content[len] == 'U') {
            p.valid = RAW_UBYTE;
          }
        }
        --len;
        break;
      case 's':
      case 'S':
        if (--len >= 0 && (str->content[len] == 'u' || str->content[len] == 'U')) {
          p.valid = RAW_USHORT;
          --len;
        } else {
          p.valid = RAW_SHORT;
        }
        break;
      case 'i':
      case 'I':
        if (--len >= 0 && (str->content[len] == 'u' || str->content[len] == 'U')) {
          p.valid = RAW_UINT;
          --len;
        } else {
          p.valid = RAW_INT;
        }
        break;
      default:
        p.valid = RAW_LONG;
        break;
    }
  } else {
    p.valid = RAW_INVALID;
  }

  if (str->content[0] == '0') {
    if (str->content[1] == 'b' || str->content[1] == 'B') {
      for (int i = len; i > 1; i--) {
        char c = str->content[i];
        if (c != '0' && c != '1') {
          p.valid = RAW_INVALID;
          break;
        }
        p.integer += (c == '1') << (len - i);
      }
    } else if (str->content[1] == 'o' || str->content[1] == 'O') {
      for (int i = len; i > 1; i--) {
        char c = str->content[i];
        if (c < '0' || c > '7') {
          p.valid = RAW_INVALID;
          break;
        }
        p.integer += (c - '0') << (4 * (len - i));
      }
    } else if (str->content[1] == 'x' || str->content[1] == 'X') {
      for (int i = len; i > 1; i--) {
        char c = str->content[i];
        if (c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'a') {
          p.valid = RAW_INVALID;
          break;
        }
        if (c <= '9') c -= '0';
        else if (c <= 'F') c -= 'A' - 10;
        else if (c <= 'f') c -= 'a' - 10;
        p.integer += c << (8 * (len - i));
      }
    } else if (str->content[0] >= '0' && str->content[0] <= '7') {
      for (int i = len; i > 0; i--) {
        char c = str->content[i];
        if (c < '0' || c > '7') {
          p.valid = RAW_INVALID;
          break;
        }
        p.integer += (c - '0') << (4 * (len - i));
      }
    } else {
      p.valid = RAW_INVALID;
    }
  } else if (str->content[0] >= '1' && str->content[0] <= '9') {
      for (int i = len; i >= 0; i--) {
        char c = str->content[i];
        if (c < '0' || c > '9') {
          p.valid = RAW_INVALID;
          break;
        }
        p.integer += (c - '0') * intpow(10, len - i);
      }
  } else {
    p.valid = RAW_INVALID;
  }
  return p;
}
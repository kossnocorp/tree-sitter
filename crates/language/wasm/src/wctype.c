#include <wctype.h>

int iswlower(wint_t wch) {
    return (unsigned)wch - L'a' < 26;
}

int iswupper(wint_t wch) {
    return (unsigned)wch - L'A' < 26;
}

int iswdigit(wint_t wch) {
  switch (wch) {
  case L'0':
  case L'1':
  case L'2':
  case L'3':
  case L'4':
  case L'5':
  case L'6':
  case L'7':
  case L'8':
  case L'9':
    return 1;
  default:
    return 0;
  }
}

int iswxdigit(wint_t wch) {
  return (unsigned)wch - '0' < 10 || (unsigned)(wch | 32) - 'a' < 6;
}

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

/**
 * getstrings: tool to dump AoC language strings from dll files
 *
 * Output strings in a ID=VALUE format, for voobly-style language.ini:
 *   getstrings /path/to/language.dll
 * Output strings in a Windows resource format, to compile into a new dll:
 *   getstrings -rc /path/to/language.dll
 */

wchar_t* escape (wchar_t* str, char quoted) {
  int i = 0;
  int j = 0;
  wchar_t escaped[8195];
  if (quoted) {
    escaped[j++] = L'"';
  }
  while (str[i] != L'\0' && j < 8193) {
    switch (str[i]) {
      case L'\r':
        escaped[j++] = L'\\';
        escaped[j++] = L'r';
        break;
      case L'\n':
        escaped[j++] = L'\\';
        escaped[j++] = L'n';
        break;
      case L'"':
        if (quoted) {
          escaped[j++] = '\\';
          escaped[j++] = str[i];
          break;
        }
        // fall through
      default:
        escaped[j++] = str[i];
        break;
    }
    i++;
  }
  if (quoted) {
    escaped[j++] = L'"';
  }
  escaped[j++] = L'\0';

  wchar_t* heap = calloc(j, sizeof(wchar_t));
  wmemcpy(heap, escaped, j);
  return heap;
}

int main (int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: getstrings ~/path/to/language.dll\n");
    return 1;
  }

  char* lib_name = argv[1];
  char as_string_table = argc > 2 && (strcmp(argv[2], "--rc") == 0 || strcmp(argv[2], "-rc") == 0);
  if (argc > 2 && !as_string_table) {
    if (strcmp(argv[1], "--rc") == 0 || strcmp(argv[1], "-rc") == 0) {
      as_string_table = 1;
      lib_name = argv[2];
    }
  }

  HINSTANCE library = LoadLibraryA(lib_name);
  if (library == NULL) {
    fprintf(stderr, "getstrings: could not load dll\n");
    return 1;
  }

  wchar_t string16[8192];
  char escaped8[16384];
  if (as_string_table) {
    wprintf(L"STRINGTABLE {\n");
  }

  for (int i = 0; i < 0xFFFF; i++) {
    int len = LoadStringW(library, i, string16, 8192);
    if (len) {
      string16[len] = L'\0';
      wchar_t* escaped16 = escape(string16, as_string_table);
      if (as_string_table) {
        wprintf(L"  %d, %s\n", i, escaped16);
      } else {
        WideCharToMultiByte(CP_UTF8, 0, escaped16, -1, escaped8, sizeof(escaped8), NULL, NULL);
        printf("%d=%s\n", i, escaped8);
      }
      free(escaped16);
    }
  }

  if (as_string_table) {
    wprintf(L"}\n");
  }
}

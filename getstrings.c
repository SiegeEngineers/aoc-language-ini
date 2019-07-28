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

wchar_t* escape (wchar_t* str) {
  int i = 0;
  int j = 0;
  wchar_t escaped[8193];
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
      default:
        escaped[j++] = str[i];
        break;
    }
    i++;
  }
  escaped[j++] = L'\0';

  wchar_t* heap = calloc(j, sizeof(wchar_t));
  wmemcpy(heap, escaped, j);
  return heap;
}

const char* hexchars = "0123456789ABCDEF";

char* encode_utf16 (wchar_t* str) {
  int i = 0;
  int j = 0;
  char escaped[32776];
  escaped[j++] = 'L';
  escaped[j++] = '"';
  while (str[i] != L'\0' && j < 32773) {
    switch (str[i]) {
      case L'\r':
        escaped[j++] = '\\';
        escaped[j++] = 'r';
        break;
      case L'\n':
        escaped[j++] = '\\';
        escaped[j++] = 'n';
        break;
      case L'"':
        escaped[j++] = '\\';
        escaped[j++] = '"';
        break;
        // fall through
      default:
        if (str[i] > 127) {
          escaped[j++] = '\\';
          escaped[j++] = 'x';
          escaped[j++] = hexchars[(str[i] & 0xF000) >> 12];
          escaped[j++] = hexchars[(str[i] & 0x0F00) >> 8];
          escaped[j++] = hexchars[(str[i] & 0x00F0) >> 4];
          escaped[j++] = hexchars[str[i] & 0x000F];
        } else {
          escaped[j++] = str[i];
        }
        break;
    }
    i++;
  }
  escaped[j++] = '"';
  escaped[j++] = '\0';

  char* heap = calloc(j, sizeof(char));
  memcpy(heap, escaped, j);
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
    printf("STRINGTABLE {\n");
  }

  for (int i = 0; i < 0xFFFF; i++) {
    int len = LoadStringW(library, i, string16, 8192);
    if (len) {
      string16[len] = L'\0';
      if (as_string_table) {
        char* encoded = encode_utf16(string16);
        printf("  %d, %s\n", i, encoded);
        free(encoded);
      } else {
        wchar_t* escaped16 = escape(string16);
        WideCharToMultiByte(CP_UTF8, 0, escaped16, -1, escaped8, sizeof(escaped8), NULL, NULL);
        printf("%d=%s\n", i, escaped8);
        free(escaped16);
      }
    }
  }

  if (as_string_table) {
    printf("}\n");
  }
}

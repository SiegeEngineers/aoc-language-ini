#define WIN32_MEAN_AND_LEAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/**
 * getstrings: tool to dump AoC language strings from dll files
 *
 * Output strings in a ID=VALUE format, for voobly-style language.ini:
 *   getstrings /path/to/language.dll
 * Output strings in a Windows resource format, to compile into a new dll:
 *   getstrings -rc /path/to/language.dll
 */

char* escape(char* str, char quoted) {
  int i = 0;
  int j = 0;
  char escaped[8195];
  if (quoted) {
    escaped[j++] = '"';
  }
  while (str[i] != '\0' && j < 8193) {
    switch (str[i]) {
    case '\r':
      escaped[j++] = '\\';
      escaped[j++] = 'r';
      break;
    case '\n':
      escaped[j++] = '\\';
      escaped[j++] = 'n';
      break;
    case '"':
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
    escaped[j++] = '"';
  }
  escaped[j++] = '\0';
  return strdup(escaped);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: getstrings ~/path/to/language.dll\n");
    return 1;
  }

  char* lib_name = argv[1];
  char as_string_table =
      argc > 2 && (strcmp(argv[2], "--rc") == 0 || strcmp(argv[2], "-rc") == 0);
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

  char string[8192];
  if (as_string_table) {
    printf("STRINGTABLE {\n");
  }

  for (int i = 0; i < 0xFFFF; i++) {
    int len = LoadStringA(library, i, string, 8192);
    if (len) {
      string[len] = '\0';
      char* escaped = escape(string, as_string_table);
      if (as_string_table) {
        printf("  %d, %s\n", i, escaped);
      } else {
        printf("%d=%s\n", i, escaped);
      }
      free(escaped);
    }
  }

  if (as_string_table) {
    printf("}\n");
  }
}

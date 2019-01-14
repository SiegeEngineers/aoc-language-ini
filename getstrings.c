#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>

char* escape (char* str) {
  int i = 0;
  int j = 0;
  char escaped[8195];
  escaped[j++] = '"';
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
        escaped[j++] = '\\';
        escaped[j++] = str[i];
        break;
      default:
        escaped[j++] = str[i];
        break;
    }
    i++;
  }
  escaped[j++] = '"';
  escaped[j++] = '\0';
  return strdup(escaped);
}

int main (int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: getstrings ~/path/to/language.dll\n");
    return 1;
  }

  HINSTANCE library = LoadLibraryA(argv[1]);
  if (library == NULL) {
    fprintf(stderr, "getstrings: could not load dll\n");
    return 1;
  }

  char string[8192];
  printf("STRINGTABLE {\n");
  for (int i = 0; i < 0xFFFF; i++) {
    int len = LoadStringA(library, i, string, 8192);
    if (len) {
      string[len] = '\0';
      char* escaped = escape(string);
      printf("  %d, %s\n", i, escaped);
      free(escaped);
    }
  }
  printf("}\n");
}

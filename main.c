#include <windows.h>
#include <stdio.h>
#include "api.h"

static char** up_mod_game_dir = (char**) (size_t) 0x7A506C;

static char file_exists (char* filename) {
  WIN32_FIND_DATA file_data;
  HANDLE exists = FindFirstFile(filename, &file_data);
  if (exists == INVALID_HANDLE_VALUE) {
    return 0;
  }
  FindClose(exists);
  return 1;
}

static void init () {
  aoc_ini_init();

  char ini_path[MAX_PATH];
  memset(ini_path, 0, sizeof(ini_path));
  if (*up_mod_game_dir[0] != '\0') {
    // up_mod_game_dir contains trailing '\'
    sprintf(ini_path, "%s%s", *up_mod_game_dir, "language.ini");
  } else {
    sprintf(ini_path, "%s", "language.ini");
  }
  if (file_exists(ini_path)) {
    aoc_ini_load_strings(ini_path);
  }
}

static void deinit () {
  aoc_ini_deinit();
}

BOOL WINAPI DllMain (HINSTANCE dll, DWORD reason, void* _) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(dll);
      init();
      break;
    case DLL_PROCESS_DETACH:
      deinit();
      break;
  }
  return 1;
}
